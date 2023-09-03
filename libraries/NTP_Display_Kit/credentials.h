/*
  NB: do not commit changes to this file which include sensitive information!
  
  Use the following command to ignore changes to this files, both local and upstream
  
  >   git update-index --skip-worktree credentials.h

  Use the following command to track changes to the file

  >   git update-index --no-skip-worktree credentials.h
*/

// add your Wi-Fi network name and password on the following lines
#define WIFI_SSID "network_name"
#define WIFI_PASSWORD "network_password"

/*
  Add your (the sender's) email username and password on the following lines. Since since May 30,
  2022, Google does not allow the use of account passwords by third-party apps. Instead, Google
  app passwords are unique 16-digit passcodes.
   
  e.g.   AUTHOR_PASSWORD "1234567890abcdef"
*/

#define AUTHOR_EMAIL "email_user.name@domain.tld"
#define AUTHOR_PASSWORD "email_password"

/* Recipient email address */
#define RECIPIENT_EMAIL "email_user.name@domain.tld"
