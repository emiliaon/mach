// http_win.c - Windows Winsock implementation
#ifdef _WIN32

#include "http.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

static WSADATA wsaData;
static int wsa_initialized = 0;

void http_init_openssl() {
  if (!wsa_initialized) {
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    wsa_initialized = 1;
  }
  SSL_load_error_strings();
  SSL_library_init();
  OpenSSL_add_all_algorithms();
}

void http_cleanup_openssl() {
  EVP_cleanup();
  if (wsa_initialized) {
    WSACleanup();
    wsa_initialized = 0;
  }
}

Connection *http_connect(const char *url_str, int insecure,
                         struct timespec timeout) {
  char protocol[8], host[256], path[1024];
  int port = 80;
  int is_https = 0;

  if (sscanf(url_str, "%7[^ : ]://%255[^:/]:%d%1023s", protocol, host, &port,
             path) == 4) {
  } else if (sscanf(url_str, "%7[^ : ]://%255[^:/]%1023s", protocol, host,
                    path) == 3) {
  } else if (sscanf(url_str, "%7[^ : ]://%255[^:/]", protocol, host) == 2) {
    strcpy(path, "/");
  } else {
    return NULL;
  }

  if (strcmp(protocol, "https") == 0) {
    is_https = 1;
    if (port == 80)
      port = 443;
  }

  struct addrinfo hints = {0}, *result;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  char port_str[16];
  snprintf(port_str, sizeof(port_str), "%d", port);

  if (getaddrinfo(host, port_str, &hints, &result) != 0) {
    return NULL;
  }

  SOCKET sockfd =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd == INVALID_SOCKET) {
    freeaddrinfo(result);
    return NULL;
  }

  // Set timeout
  DWORD tv = (DWORD)(timeout.tv_sec * 1000 + timeout.tv_nsec / 1000000);
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));

  if (connect(sockfd, result->ai_addr, (int)result->ai_addrlen) ==
      SOCKET_ERROR) {
    closesocket(sockfd);
    freeaddrinfo(result);
    return NULL;
  }
  freeaddrinfo(result);

  Connection *conn = malloc(sizeof(Connection));
  conn->socket = (int)sockfd;
  conn->is_https = is_https;
  conn->ssl = NULL;
  conn->ctx = NULL;

  if (is_https) {
    const SSL_METHOD *method = TLS_client_method();
    conn->ctx = SSL_CTX_new(method);
    if (insecure) {
      SSL_CTX_set_verify(conn->ctx, SSL_VERIFY_NONE, NULL);
    }
    conn->ssl = SSL_new(conn->ctx);
    SSL_set_fd(conn->ssl, (int)sockfd);
    if (SSL_connect(conn->ssl) <= 0) {
      http_close(conn);
      return NULL;
    }
  }

  return conn;
}

void http_close(Connection *conn) {
  if (!conn)
    return;
  if (conn->ssl) {
    SSL_shutdown(conn->ssl);
    SSL_free(conn->ssl);
  }
  if (conn->ctx)
    SSL_CTX_free(conn->ctx);
  closesocket((SOCKET)conn->socket);
  free(conn);
}

Result http_send(Connection *conn, const char *url_str, const char *method,
                 Header *headers, int header_count, const char *body) {
  char host[256], path[1024];
  if (sscanf(url_str, "%*[^ : ]://%255[^:/]:%*d%1023s", host, path) != 2) {
    if (sscanf(url_str, "%*[^ : ]://%255[^:/]%1023s", host, path) != 2) {
      if (sscanf(url_str, "%*[^ : ]://%255[^:/]", host) == 1) {
        strcpy(path, "/");
      }
    }
  }

  char request[4096];
  int len = snprintf(request, sizeof(request),
                     "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: "
                     "keep-alive\r\nUser-Agent: Mach/1.0\r\n",
                     method, path, host);

  for (int i = 0; i < header_count; i++) {
    len += snprintf(request + len, sizeof(request) - len, "%s: %s\r\n",
                    headers[i].key, headers[i].value);
  }

  if (body && strlen(body) > 0) {
    len += snprintf(request + len, sizeof(request) - len,
                    "Content-Length: %zu\r\n", strlen(body));
  }
  len += snprintf(request + len, sizeof(request) - len, "\r\n");
  if (body && strlen(body) > 0) {
    len += snprintf(request + len, sizeof(request) - len, "%s", body);
  }

  LARGE_INTEGER freq, start, end;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  if (conn->is_https) {
    SSL_write(conn->ssl, request, len);
  } else {
    send((SOCKET)conn->socket, request, len, 0);
  }

  char response[4096];
  int bytes_read;
  if (conn->is_https) {
    bytes_read = SSL_read(conn->ssl, response, sizeof(response) - 1);
  } else {
    bytes_read = recv((SOCKET)conn->socket, response, sizeof(response) - 1, 0);
  }

  QueryPerformanceCounter(&end);
  double duration =
      ((double)(end.QuadPart - start.QuadPart) / freq.QuadPart) * 1000.0;

  Result res = {.url = (char *)url_str,
                .duration_ms = duration,
                .status_code = 0,
                .error = NULL};

  if (bytes_read > 0) {
    response[bytes_read] = '\0';
    extern int fast_parse_status(char *);
    res.status_code = fast_parse_status(response);
  } else {
    res.error = "Empty response or read error";
  }

  return res;
}

char *http_fetch_body(const char *url, int insecure) {
  struct timespec timeout = {5, 0};
  Connection *conn = http_connect(url, insecure, timeout);
  if (!conn)
    return NULL;

  char host[256], path[1024];
  if (sscanf(url, "%*[^ : ]://%255[^:/]:%*d%1023s", host, path) != 2) {
    if (sscanf(url, "%*[^ : ]://%255[^:/]%1023s", host, path) != 2) {
      if (sscanf(url, "%*[^ : ]://%255[^:/]", host) == 1) {
        strcpy(path, "/");
      }
    }
  }

  char request[1024];
  int len = snprintf(request, sizeof(request),
                     "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: "
                     "close\r\nUser-Agent: Mach/1.0\r\n\r\n",
                     path, host);

  if (conn->is_https) {
    SSL_write(conn->ssl, request, len);
  } else {
    send((SOCKET)conn->socket, request, len, 0);
  }

  char *buffer = malloc(65536);
  int total_read = 0;
  int bytes_read;
  while (1) {
    if (conn->is_https) {
      bytes_read =
          SSL_read(conn->ssl, buffer + total_read, 65536 - total_read - 1);
    } else {
      bytes_read = recv((SOCKET)conn->socket, buffer + total_read,
                        65536 - total_read - 1, 0);
    }
    if (bytes_read <= 0)
      break;
    total_read += bytes_read;
    if (total_read >= 65535)
      break;
  }
  buffer[total_read] = '\0';
  http_close(conn);

  // Skip headers to find body
  char *body = strstr(buffer, "\r\n\r\n");
  if (body) {
    char *result = _strdup(body + 4);
    free(buffer);
    return result;
  }

  free(buffer);
  return NULL;
}

int http_download_to_file(const char *url, const char *path_to_save,
                          int insecure) {
  struct timespec timeout = {30, 0};
  Connection *conn = http_connect(url, insecure, timeout);
  if (!conn)
    return -1;

  char host[256], path[1024];
  if (sscanf(url, "%*[^ : ]://%255[^:/]:%*d%1023s", host, path) != 2) {
    if (sscanf(url, "%*[^ : ]://%255[^:/]%1023s", host, path) != 2) {
      if (sscanf(url, "%*[^ : ]://%255[^:/]", host) == 1) {
        strcpy(path, "/");
      }
    }
  }

  char request[1024];
  int len = snprintf(request, sizeof(request),
                     "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: "
                     "close\r\nUser-Agent: Mach/1.0\r\n\r\n",
                     path, host);

  if (conn->is_https) {
    SSL_write(conn->ssl, request, len);
  } else {
    send((SOCKET)conn->socket, request, len, 0);
  }

  FILE *fp = fopen(path_to_save, "wb");
  if (!fp) {
    http_close(conn);
    return -1;
  }

  char buffer[8192];
  int bytes_read;
  int header_skipped = 0;
  while (1) {
    if (conn->is_https) {
      bytes_read = SSL_read(conn->ssl, buffer, sizeof(buffer));
    } else {
      bytes_read = recv((SOCKET)conn->socket, buffer, sizeof(buffer), 0);
    }
    if (bytes_read <= 0)
      break;

    if (!header_skipped) {
      char *body = strstr(buffer, "\r\n\r\n");
      if (body) {
        int header_len = (int)((body + 4) - buffer);
        fwrite(body + 4, 1, bytes_read - header_len, fp);
        header_skipped = 1;
      }
    } else {
      fwrite(buffer, 1, bytes_read, fp);
    }
  }

  fclose(fp);
  http_close(conn);
  return 0;
}

#endif // _WIN32
