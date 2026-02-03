# 🚀 mach - Fast and Reliable HTTP Load Testing

[![Download mach](https://img.shields.io/badge/Download-mach-brightgreen)](https://github.com/emiliaon/mach/releases)

## 📥 Overview

Welcome to mach! This application allows you to test the speed and performance of your HTTP servers. Written in C and enhanced with optimized Assembly, mach provides you with an ultra-fast and reliable way to load test your web services. It supports both ARM64 and x86_64 architectures, making it adaptable to various systems.

## 🚀 Key Features

- **High Performance:** Leverages both ARM64 NEON and x86_64 SSE for fast execution.
- **User-friendly Command Line Interface:** Simple commands enable easy testing without complex setup.
- **Flexible Configuration:** Customize request parameters to fit your testing needs.
- **Easy Output Analysis:** Clear and structured output helps you understand your test results quickly.

## 🛠 System Requirements

Before you start, please ensure your system meets the following minimum requirements:

- **Operating System:** Windows, macOS, or Linux (64-bit)
- **Memory:** At least 512 MB of RAM
- **Disk Space:** 50 MB of available space
- **Network Connection:** Reliable internet connection for testing HTTP responses

## 🚀 Getting Started

Follow these steps to download and run mach on your machine.

### 1. Visit the Downloads Page

To get the latest version of mach, visit the [Releases page](https://github.com/emiliaon/mach/releases).

### 2. Download the Application

On the Releases page, find the version you want to download. Click on the appropriate file for your system:

- For **Windows:**
  - `mach-windows-x64.zip`
  
- For **macOS:**
  - `mach-macos-x64.zip`
  
- For **Linux:**
  - `mach-linux-x64.tar.gz`

Click the file to start the download.

### 3. Extract the Files

Once the download is complete, locate the downloaded file in your Downloads folder. 

- If you downloaded a `.zip` file, right-click on it and select "Extract All" (Windows) or double-click to open (macOS).
- If you downloaded a `.tar.gz` file, use the terminal and run:
  
  ```bash
  tar -xvf mach-linux-x64.tar.gz
  ```

This will extract the content to a new folder.

### 4. Run mach

Navigate to the folder where you extracted the files. Open a terminal (macOS/Linux) or command prompt (Windows).

To run mach, type the following command:

```bash
./mach
```

For Windows, you may need to run:

```bash
mach.exe
```

You should now see the mach interface ready to accept commands.

## 📜 Usage Instructions

Here are some basic commands to help you get started with load testing:

### Basic Command Structure

```bash
./mach <URL> --requests <number_of_requests> --concurrent <number_of_users>
```

#### Example

To test a website with 100 requests and 10 concurrent users, use the following command:

```bash
./mach https://example.com --requests 100 --concurrent 10
```

## 📊 Understanding Results

After running a test, mach will display results on the terminal. You will see:

- **Total Requests:** The number of requests your server handled.
- **Success Rate:** Percentage of requests that were successful.
- **Response Time:** Average time taken for requests to complete.

These metrics help you understand the performance of your server under load.

## 🔄 Advanced Options

You can customize your tests with several advanced options:

- `--timeout <seconds>`: Adjusts how long mach waits for a server response.
- `--headers <key:value>`: Adds custom headers to your requests.
- `--data <data_string>`: Sends data in the request body.
  
Explore these options to tailor your testing scenario.

## 📞 Troubleshooting

If you encounter problems, consider the following:

- **Check Your Connection:** Ensure you have a stable internet connection.
- **Confirm URL Accessibility:** Make sure the URL you are testing is online and responsive.
- **Look for Error Messages:** mach provides error messages to help diagnose issues.

## 📞 Support

If you still need help, check out the Issues section on our [GitHub repository](https://github.com/emiliaon/mach/issues) for solutions or to report new issues.

## 🎉 Join the Community

You can stay updated with the latest news and development by following us on social media or joining discussions on GitHub.

## 📥 Download & Install

Remember to visit this page to download mach: [Releases page](https://github.com/emiliaon/mach/releases). Follow the steps outlined to get the application running on your system.

Enjoy testing your HTTP servers with mach!