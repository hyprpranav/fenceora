// ============================================
// EMAIL SENDING FUNCTION (INDEPENDENT OF ESP32)
// ============================================
// This function sends test emails directly from the browser
// No ESP32 connection required!

async function sendTestEmail() {
  // Get email configuration
  const config = typeof EMAIL_CONFIG !== 'undefined' ? EMAIL_CONFIG : null;
  
  // Check if EmailJS is configured and enabled
  if (config && config.emailjs.enabled && config.emailjs.serviceId !== 'YOUR_SERVICE_ID') {
    try {
      // Initialize EmailJS with public key
      emailjs.init(config.emailjs.publicKey);
      
      // Send email to each receiver individually
      const emailPromises = config.recipients.receivers.map(async (receiverEmail, index) => {
        const templateParams = {
          to_email: receiverEmail,
          to_name: receiverEmail.split('@')[0], // Use email username instead of "Receiver #3"
          from_name: config.template.from_name,
          subject: config.template.subject,
          message: config.template.test_message + `\n\nThis test email was sent on ${new Date().toLocaleString()}.`,
          timestamp: new Date().toLocaleString(),
          system_status: 'Email System Active',
          alert_type: 'Test Notification',
          sender_email: config.recipients.sender,
          receiver_number: receiverEmail // Use email instead of number
        };
        
        // Send email via EmailJS
        return emailjs.send(
          config.emailjs.serviceId,
          config.emailjs.templateId,
          templateParams
        );
      });
      
      // Wait for all emails to be sent
      const responses = await Promise.all(emailPromises);
      
      console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
      console.log('✅ ALL EMAILS SENT SUCCESSFULLY!');
      console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
      console.log('From:', config.recipients.sender);
      console.log('Sent to:');
      config.recipients.receivers.forEach((email, i) => {
        console.log(`  ${i + 1}. ${email} - Status: ${responses[i].status}`);
      });
      console.log('Subject:', config.template.subject);
      console.log('Time:', new Date().toLocaleString());
      console.log('Total emails sent:', responses.length);
      console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
      
      return { success: true, responses };
      
    } catch (error) {
      console.error('❌ Email sending failed:', error);
      throw new Error('Failed to send email: ' + (error.text || error.message));
    }
  } else {
    // Simulation mode - EmailJS not configured
    return new Promise((resolve) => {
      setTimeout(() => {
        console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        console.log('📧 TEST EMAIL NOTIFICATION (SIMULATED)');
        console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        console.log('From: FENCEORA System');
        console.log('To: 4 Receivers (Simulated)');
        console.log('Subject: FENCEORA Test Notification');
        console.log('Message: Test email verification');
        console.log('Time:', new Date().toLocaleString());
        console.log('Status: SUCCESS (Simulated)');
        console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        console.log('📝 To send REAL emails:');
        console.log('1. Edit email-config.js');
        console.log('2. Set up EmailJS at https://emailjs.com');
        console.log('3. Add your credentials to config');
        console.log('4. Set enabled: true');
        console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━');
        resolve({ success: true });
      }, 2000);
    });
  }
}
