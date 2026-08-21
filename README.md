PostMail
========

PostMail is a program for sending email to Microsoft Office 365. 
One of its key features is that it 's independent of external email programs like Microsoft Office, Outlook, or MS Mail. 
A major advantage is that the program is specifically designed to interact and communicate with other desktop programs.
In addition to Office 365, the legacy protocol SMTP (Simple Mail Transport Protocol) is also still supported.

Sending the email
-----------------
This can be done in three different ways. Briefly summarized, these three ways are:

* 1st way

Launch PostMail.exe. A blank draft email message will be created and displayed in your default browser within Office 365. 
You can then finish and send the email there.
In the case of SMTP mail, PostMail itself shows you a dialog where you can create an email.

* 2nd way

An email is defined by defining it in a separate text file (with a *. txt extension). 
This file is passed as a parameter to PostMail.exe. 
PostMail reads from the definition file what it should do and then performs one of the following actions:

a - The email will be immediately sent;

b - A draft email is being created;

c - The email will be displayed within Office 365 where you can complete and send the email;

d - The email is displayed in a PostMail dialog, from which you can complete, finalize, and send the email;

e - The email will be sent immediately, depending on the definition in the file.

Which action is taken is dependent on the contents of the *.txt file


* 3rd way

The email content can be passed to PostMail via the operating system's command line and stored in the email cache. 
The central concept of the email ID links the email data. 
For each ID, the data is stored in the user's roaming profile directory. 
After the email is sent, the stored data is deleted.

