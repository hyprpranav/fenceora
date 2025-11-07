// ============================================
// FENCEORA EMAIL CONFIGURATION
// ============================================
// This file contains email service configuration
// The test email feature works INDEPENDENTLY of ESP32

// ============================================
// OPTION 1: EmailJS (Recommended - Free & Easy)
// ============================================
// 1. Go to https://www.emailjs.com/
// 2. Create a free account
// 3. Add an email service (Gmail recommended)
// 4. Create an email template
// 5. Copy your credentials below

const EMAIL_CONFIG = {
  // EmailJS Configuration
  emailjs: {
    enabled: true, // Changed to true for immediate use
    serviceId: 'service_mf3wawv',     // EmailJS Service ID
    templateId: 'template_zn9ndnt',   // EmailJS Template ID
    publicKey: '8_VkHFChgAm9XPDas'    // EmailJS Public Key
  },
  
  // Email Recipients (Your 4 receivers + 1 sender)
  recipients: {
    sender: 'fenceora.h4@gmail.com',
    receivers: [
      'harishwarharishwar47@gmail.com',
      'hemanathan8337@gmail.com',
      'harishpranavs259@gmail.com',
      'hariprasanthgk1@gmail.com'
    ]
  },
  
  // Email Template
  template: {
    subject: '🧪 FENCEORA Test Notification',
    from_name: 'FENCEORA Security System',
    test_message: 'This is a test email to verify the notification system is working properly.'
  }
};

// ============================================
// EMAILJS TEMPLATE EXAMPLE
// ============================================
// Create a template in EmailJS with these variables:
// 
// Subject: {{subject}}
// 
// Hello {{to_name}},
// 
// {{message}}
// 
// Alert Type: {{alert_type}}
// Timestamp: {{timestamp}}
// System Status: {{system_status}}
// 
// This is an automated message from the FENCEORA Security System.
// 
// Best regards,
// {{from_name}}
// ============================================

// Export configuration
if (typeof module !== 'undefined' && module.exports) {
  module.exports = EMAIL_CONFIG;
}
