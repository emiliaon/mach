import './style.css'

// Copy to clipboard functionality
const copyBtn = document.getElementById('copy-btn');
if (copyBtn) {
  copyBtn.addEventListener('click', () => {
    const text = 'mach -n 1000 -c 50 http://example.com';
    navigator.clipboard.writeText(text).then(() => {
      copyBtn.innerText = '✅';
      setTimeout(() => {
        copyBtn.innerText = '📋';
      }, 2000);
    });
  });
}

// Scroll revealing animations
const observerOptions = {
  threshold: 0.1
};

const observer = new IntersectionObserver((entries) => {
  entries.forEach(entry => {
    if (entry.isIntersecting) {
      entry.target.classList.add('reveal');
    }
  });
}, observerOptions);

document.querySelectorAll('.feature-card, .split-view, .install-box').forEach(el => {
  el.style.opacity = '0';
  el.style.transform = 'translateY(20px)';
  el.style.transition = 'opacity 0.6s ease-out, transform 0.6s ease-out';
  observer.observe(el);
});

// Implementation of reveal class
const style = document.createElement('style');
style.textContent = `
  .reveal {
    opacity: 1 !important;
    transform: translateY(0) !important;
  }
`;
document.head.appendChild(style);
