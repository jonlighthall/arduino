/*
  NB: do not commit changes to this file which include sensitive information!
  
  Use the following command to ignore changes to this files, both local and upstream
  
  >   git update-index --skip-worktree credentials.h

  Use the following command to track changes to the file

  >   git update-index --no-skip-worktree credentials.h
*/

/**
 * This file contains excerpts from Send_Text.ino created by K. Suwatchai (Mobizt)
 * Github: https://github.com/mobizt/ESP-Mail-Client
 * Copyright (c) 2021, 2023 mobizt
 */

// add your Wi-Fi network name and password on the following lines
#define WIFI_SSID "network_name"
#define WIFI_PASSWORD "network_password"

/*
  Add your (the sender's) email username and password on the following lines. Since since May 30,
  2022, Google does not allow the use of account passwords by third-party apps. Instead, Google
  will generate unique 16-digit passcodes called app passwords. To use Gmail's App Password to
  sign in, define the AUTHOR_PASSWORD with your App Password and AUTHOR_EMAIL with your account
  email.
   
  e.g.   AUTHOR_PASSWORD "1234567890abcdef"
*/

#define AUTHOR_EMAIL "email_user.name@domain.tld"
#define AUTHOR_PASSWORD "email_password"

/* Recipient email address */
#define RECIPIENT_EMAIL "email_user.name@domain.tld"

void add_recip(SMTP_Message &message) {
  //message.addRecipient(F("Someone"), RECIPIENT_EMAIL);
}
