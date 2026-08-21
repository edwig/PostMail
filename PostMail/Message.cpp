////////////////////////////////////////////////////////////////////////////////
//
// SourceFile: Messages.cpp
//
// Mailer to post an e-mail to the DARPA-net
// 
// Written by W.E. Huisman (2006-2026)
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "stdafx.h"
#include "message.h"

// Define your preferred default language in your project settings
// Otherwise the compilation will default to the ENGLISH language

// Global language code = g_lang

#ifdef DEF_DUTCH
int g_lang = LANGUAGE_NEDERLANDS;
#endif
#ifdef DEF_FRANCAIS
int g_lang = LANGUAGE_FRANCAIS
#endif
#ifdef DEF_DEUTSCH
int g_lang = LANGUAGE_DEUTSCH
#endif

#ifndef DEF_DUTCH 
#ifndef DEF_FRANCAIS
#ifndef DEF_DEUTSCH
int g_lang = LANGUAGE_ENGLISH;
#endif
#endif
#endif

TCHAR* g_message[][4] = 
{
  {// MESS_OK
     _T("OK")
    ,_T("OK")
    ,_T("D'ACCORD")
    ,_T("OK")
  },
  {//MESS_WINSOCK
    _T("Kan de MS-Windows fitting bibliotheken (WinSocket) niet laden")
   ,_T("Failed to initialise the Windows socket library")
   ,_T("Impossible de initialisee la libraire de soquet (WinSocket)")
   ,_T("Nicht möglich die Fassungs Bibliotheken (WinSocket) zu initialisieren")
  },
  {//MESS_PARAMETER
    _T("U dient een e-mail definitie bestand als parameter op te geven!")
   ,_T("You must provide an e-mail definition file as a parameter!")
   ,_T("Il faut indiquer une fichier de definitions e-mail!")
   ,_T("Sie mussen ein E-mail definitions Datei mitgeben!")
  },
  {//MESS_FAILCONNECT
    _T(": Kan geen verbinding met de server maken: ")
   ,_T(": Failed to connect to server: ")
   ,_T(": Ne pas connecté avec la serveur: ")
   ,_T(": Keine verbindung mit den SMTP server: ")
  },
  {//MESS_FAILSEND
    _T("Kon het e-mail bericht niet versturen")
   ,_T("Failed to send the e-mail message")
   ,_T("Ne pas réussi de envoyer la message electronique")
   ,_T("Die E-mail is nicht versonden")
  },
  {//MESS_CONFIRM
    _T("Mail boodschap %sverstuurd naar: %s")
   ,_T("Mail message was %ssent to: %s")
   ,_T("Message %senvoyer a: %s")
   ,_T("E-mail ist %sversonden an: %s")
  },
  {//MESS_NOT
    _T("NIET ")
   ,_T("NOT ")
   ,_T("NE PAS ")
   ,_T("NICHT ")
  },
  {//MESS_STARTLOG
    _T("Start van het logbestand van PostMail")
   ,_T("Start of the logfile of PostMail")
   ,_T("Lancer le carnet de PostMail")
   ,_T("Anfang des logbuchs von PostMail")
  },
  {//MESS_ENDLOG
    _T("Einde van het logboek van PostMail")
   ,_T("End of the logfile of PostMail")
   ,_T("Fin de carnet de PostMail")
   ,_T("Ende des logbuchs von PostMail")
  },
  {//MESS_NOFILE
    _T("Kan de status niet bepalen voor bestand: %s.\nHet bestaat waarschijnlijk niet, of is nog geopend in een viewer.")
   ,_T("Can't determine the status for file: %s.\nIt probably doesn't exist, or is still open in a viewer.")
   ,_T("Impossible de déterminer le statut du fichier: %s\nIl n'existe probablement pas, ou est encore ouvert dans une visionneuse.")
   ,_T("Kann den Status für die Datei [%s] nicht bestimmen\ndie wahrscheinlich nicht existiert oder noch in einem Viewer geöffnet ist.")
  },
  {//MESS_OPENFILE
    _T("Kan bestand voor bijlagen niet openen: %s\nHet bestaat waarschijnlijk niet, of is nog geopend in een viewer")
   ,_T("Failed to open file to be attached: %s.\nIt probably doesn't exist, or is still open in a viewer.")
   ,_T("Impossible d'ouvrir le fichier pour les pièces jointes : %s\nIl n'existe probablement pas, ou est encore ouvert dans une visionneuse")
   ,_T("Kann Datei für Anhänge nicht öffnen: %s\nVermutlich nicht vorhanden, oder noch in einem Viewer geöffnet")
  },
  {//MESS_MAILADDRESS
    _T("Een fout in een e-mail adres. Moet zijn: \"Mooie naam\"<uw.adres@voorbeeld.com>")
   ,_T("An error occurred while parsing the recipients string. Must be: \"Nice name\"<your.address@example.com>")
   ,_T("Faute dans l'address electronique. Il doit etre: \"Jolie Nome\"<votre.adresse@example.com")
   ,_T("Eine fehler in die E-mail Adresse. Muss sein: \"Schöne Name\"<ihre.adresse@beispiel.de")
  },
  {//MESS_MAILTXT
    _T("Kan het definitiebestand niet lezen: ")
   ,_T("Cannot read the mail message file: ") 
   ,_T("Ne pas possible de lire la fichier de definition: ")
   ,_T("Nicht möglich die definitions Datei zu öffnen: ")
  },
  {//MESS_UNKNOWNLINE
    _T("Onbekend type regel in het definitie bestand")
   ,_T("Unknown line type in the message definition file")
   ,_T("Linie inconnu dans le Fichier de defintion")
   ,_T("Unbekantes linie in die definitions Datei")
  },
  {//MESS_SEND
    _T("&Verzenden")
   ,_T("&Send e-mail")
   ,_T("&Envoyer")
   ,_T("&Senden")
  },
  {//MESS_CANCEL
    _T("&Annuleren")
   ,_T("&Cancel")
   ,_T("&Annuler")
   ,_T("&Annulieren")
  },
  {//MESS_TO
    _T("Zenden aan")
   ,_T("Send to")
   ,_T("Envoyer a")
   ,_T("Senden an")
  },
  {//MESS_CC
    _T("CC Zenden")
   ,_T("Send CC")
   ,_T("Envoyer CC")
   ,_T("CC Senden")
  },
  {//MESS_SUBJECT
    _T("Onderwerp")
   ,_T("Subject")
   ,_T("Sujet")
   ,_T("Thema")
  },
  {//MESS_TEXT
    _T("Boodschap")
   ,_T("Text")
   ,_T("Communique")
   ,_T("Nachricht")
  },
  {//MESS_ATTACH
    _T("Bijlagen")
   ,_T("Attachments")
   ,_T("Annexes")
   ,_T("Anlagen")
  },
  {//MESS_NOHOST
    _T("Geen SMTP mail-host opgegeven die de mail moet gaan versturen! (HOST:)")
   ,_T("No SMTP mail host to send the mail from (HOST:)")
   ,_T("Pas de SMTP e-mail 'host' qui relay la message electronique (HOST:)")
   ,_T("Keine SMTP mail 'host' der die E-mail senden muss (HOST:)")
  },
  {//MESS_NOSENDER
    _T("Geen verzender van de mail opgegeven! (FROM:)")
   ,_T("No sender of the mail defined! (FROM:)")
   ,_T("Pas d'une origine de la message electronique (FROM:)")
   ,_T("Keine sender des e-mails gefunden (FROM:)")
  },
  {//MESS_NORCPT
    _T("Niet minimaal 1 geaddresseerde(n) opgegeven! (TO/CC/BCC)")
   ,_T("Not at least 1 recepient addres defined (TO/CC/BCC)")
   ,_T("Pas d'une minimum d'une recepient de la message electronique (TO/CC/BCC)")
   ,_T("Nicht minimal 1 addressierte für die E-mail (TO/CC/BCC)")
  },
  {//MESS_NOSUBJECT
    _T("Geen onderwerp opgegeven! (SUBJECT:)")
   ,_T("No subject given! (SUBJECT:)")
   ,_T("Pas de suject definee! (SUBJECT:)")
   ,_T("Keine themen definiert! (SUBJECT:)")
  },
  {//MESS_NOBODY
    _T("De mail bevat geen bericht! (<BODY>)")
   ,_T("The e-mail does not contain a body! (<BODY>)")
   ,_T("La message electronique se manque une texte! (<BODY>)")
   ,_T("Die E-mail hat keine bericht! (<BODY>)")
  },
  {//MESS_NOSOCKET
    _T("Kan geen verbinding maken met de opgegeven SMTP server")
   ,_T("Failed to connect to the specified SMTP server")
   ,_T("Impossible de creée une connection avec le serveur SMTP")
   ,_T("Nicht möglich eine verbindung mit die SMTP server zu machen")
  },
  {//MESS_NOCONNECT
    _T("Kan geen verbinding maken met de SMTP server %s op poort %d")
   ,_T("Could not connect to the SMTP server %s on port %d")
   ,_T("Impossible de connecté le SMTP serveur %s a la porte %d")
   ,_T("Nicht möglich die SMTP server %s auf porte %d zu bereichen")
  },
  {//MESS_UNEX_LOGON
    _T("Fout bij aanmelden op de SMTP server: Een onverwacht SMTP connectie antwoord ontvangen")
   ,_T("Error at connecting to SMTP server: An unexpected SMTP connection response was received")
   ,_T("Faute dans la connection à la SMTP serveur: Reçu une response n'attendee pas a la connection")
   ,_T("Fehler bei anmeldung an der SMTP server: Eine nicht erwartette antwort bij der anmeldung an der SMTP server")
  },
  {//MESS_FAIL_QUIT
    _T("Zenden van het QUIT commando is mislukt")
   ,_T("Failed in call to send QUIT command")
   ,_T("Echouer de envoyer la commande QUIT")
   ,_T("Senden des QUIT kommandos is nicht gelungen")
  },
  {//MESS_UNEX_QUIT
    _T("Fout bij uitloggen van de SMTP server: Een onverwacht antwoord op het QUIT commando ontvangen")
   ,_T("Error in logoff from SMTP server: an unexpected QUIT response was received")
   ,_T("Faute a la logg-off de la SMTP serveur: Reçu une response n'attendee pas a la commande QUIT")
   ,_T("Fehler bei das ausloggen der SMTP server: Ein nicht erwartette antwort auf das QUIT kommando") 
  },
  {//MESS_QUIT_ALREADY
    _T("De verbinding met de SMTP server was al verbroken, we doen niets meer")
   ,_T("Already disconnected from the SMTP server, doing nothing")
   ,_T("La connection avec la serveur SMTP est deja rompre")
   ,_T("Die verbindung mit der SMTP server ist schon verbrochen")
  },
  {//MESS_FAIL_MAIL
    _T("Zenden van het 'MAIL FROM' comamndo is mislukt")
   ,_T("Failed in call to send 'MAIL FROM' command")
   ,_T("Echouer de envoyer la commande 'MAIL FROM'")
   ,_T("Senden des 'MAIL FROM' kommandos is nicht gelungen")
  },
  {//MESS_UNEX_MAIL
    _T("Fout in het mail-verzender addres: Een onverwacht antwoord van het 'MAIL FROM' commando ontvangen")
   ,_T("Error in mail-senders address: an unexpected response to the 'MAIL FROM' command")
   ,_T("Faute en l'addresse de l'envoyeur: Reçu une response n'attendee pas a la commande 'MAIL FROM'")
   ,_T("Fehler in das address des Senders: Ein nicht erwartette antwort auf das 'MAIL FROM' kommando")
  },
  {//MESS_FAIL_RCPT
    _T("Zenden van het 'RCPT TO' commando is mislukt")
   ,_T("Failed in call to send RCPT TO command")
   ,_T("Echouer de envoyer la commande 'RCPT TO'")
   ,_T("Senden des 'RCPT TO' kommandos is nicht gelungen")
  },
  {//MESS_UNEX_RCPT
    _T("Het adres van de ontvanger werd niet geaccepteerd:")
   ,_T("Recepients address not accepted:")
   ,_T("L'addresse de l'receptioniste nes pas accepté:")
   ,_T("Die adresse der Empfänger wirt nicht acceptiert:")
  },
  {//MESS_LASTBUFFER
    _T("Laatst bekende ontvangst buffer:")
   ,_T("Last known received buffer:")
   ,_T("Dernier connu depot de reception:")
   ,_T("Letzte bekantesten empfangenen puffer:")
  },
  {//MESS_TIMEOUT
    _T("Time-out opgetreden op de SMTP server")
   ,_T("SMTP Server timeout occured")
   ,_T("Time-out a la serveur SMTP")
   ,_T("Time-out aufgetreten auf der SMTP server")
  },
  {//MESS_LOST_CONNECTION
    _T("SMTP kanaal naar server niet bereikbaar of verbinding verloren gegaan")
   ,_T("SMTP Server channel was not accessible or suffered from lost connection")
   ,_T("Canal de SMTP serveur inaccesible ou perdu connection")
   ,_T("SMTP kanal nicht erreichbar oder die verbindung verloren")
  },
  {//MESS_LOST_SOCKET
    _T("SMTP kanaal: Fout in socket, verbinding verloren")
   ,_T("SMTP Server channel : socket error, connection lost")
   ,_T("Canal de SMTP serveur: faute en socket, perdu connection")
   ,_T("SMTP kanal: socket fehler, verbindung verloren")
  },
  {//MESS_FAIL_DATA
    _T("Zenden van het 'MAIL' commando is mislukt")
   ,_T("Failed in call to send 'MAIL' command")
   ,_T("Echouer de envoyer la commande 'MAIL'")
   ,_T("Senden des 'MAIL' kommandos ist nicht gelungen")
  },
  {//MESS_UNEX_DATA
    _T("Fout bij versturen mail tekst: een onverwacht antwoord op het 'DATA' commando ontvangen")
   ,_T("Error in the body of the mail: an unexpected DATA response was received")
   ,_T("Faute dans la texte de la message electronique: Reçu une response n'attandee pas a la commande 'DATA'")
   ,_T("Fehler in das senden von die E-mail tekste: Ein nicht erwartette antwort auf das 'DATA' kommando")
  },
  {//MESS_FAIL_HEADER
    _T("Zenden van de koptekst van de mail is mislukt")
   ,_T("Failed in call to send the header")
   ,_T("Echouer de envoyer le en-tête de la message electronique")
   ,_T("Fehler in das senden der kopftekst des E-mails")
  },
  {//MESS_FAIL_MDN
    _T("Fout bij het versturen van de antwoordbevestiging koptekst")
   ,_T("Failed in call to send mail-disposition-notification header")
   ,_T("Echouer de envoyer la en-tête de la disposition-repondre")
   ,_T("Fehler in das senden der kopftekst von die antwort-bestätigung")
  },
  {//MESS_FAIL_BODYHEADER
    _T("Zenden van de koptekst van het bericht is mislukt")
   ,_T("Failed in call to send the body header")
   ,_T("Echouer de envoyer le en-tête de la texte de la message electronique")
   ,_T("Fehler in das senden der kopftekst von die tekste des E-mails")
  },
  {//MESS_FAIL_BODY
    _T("Zenden van de tekst van het bericht is mislukt")
   ,_T("Failed in call to send the body of the message")
   ,_T("Echouer de envoyer la texte de la message electronique")
   ,_T("Fehler in das senden von die tekste des E-mails")
  },
  {//MESS_FAIL_MIMEHEADER
    _T("Zenden van de MIME-bijlage-koptekst is mislukt")
   ,_T("Failed in call to send MIME attachment header")
   ,_T("Echouer de envoyer le en-tête pour le annexe MIME")
   ,_T("Fehler in das senden von die kopftekst von die MIME anlage")
  },
  {//MESS_FAIL_ATTACH
    _T("Zenden van de bijlage is mislukt")
   ,_T("Failed in call to send the attachment")
   ,_T("Echouer de envoyer l'annexe")
   ,_T("Fehler in das senden von die anlage")
  },
  {//MESS_FAIL_MIMEFOOTER
    _T("Zenden van de MIME-bijlage-voetnoot is mislukt")
   ,_T("Failed in call to send MIME attachment footer")
   ,_T("Echouer de envoyer la note de bas de page MIME")
   ,_T("Fehler in das senden von die fussnote MIME")
  },
  {//MESS_FAIL_EOM
    _T("Zenden van einde-bericht-indicator is mislukt")
   ,_T("Failed in call to send end-of-message indicator")
   ,_T("Echouer de envoyer la indice fin-de-message")
   ,_T("Fehler in das senden von die ende-bericht-indikator")
  },
  {//MESS_UNEX_EOM
    _T("Fout bij het einde van het mail bericht: Een onverwacht antwoord op het 'einde-bericht' commando")
   ,_T("Error at the end of the mail body: An unexpected end of message response was received")
   ,_T("Faute en la fin de la message: Reçu response n'attende pas a la commande 'fin-de-message'")
   ,_T("Fehler an das ende des berichts: Nicht erwarttete antwort auf das 'ende-bericht' kommando")
  },
  {//MESS_NOADDRESS
    _T("Een leeg e-mail adres is niet toegestaan")
   ,_T("An empty e-mail address is not allowed")
   ,_T("Un adresse electronique vide n'est pas permette")
   ,_T("Ein leeres adresse ist nicht erlaubt")
  },
  {//MESS_NOFILENAME
    _T("Een lege bestandsnaam voor een bijlage is niet toegestaan")
   ,_T("An empty filename for an attachment is not allowed")
   ,_T("Une vide nome de fichier n'est pas permette")
   ,_T("Eine leere Name für ein Datei ist nicht erlaubt")
  },
  {//MESS_NOMEM_ATTACH
    _T("Onvoldoende geheugen om de bijlage in MIME-Base64 te coderen")
   ,_T("Not enough memory to encode the attachment in MIME-Base64")
   ,_T("Insufficiante mémoire pour le encode en MIME-Base64 de la annexe")
   ,_T("Datenspeicher is nicht ausreichend für das MIME-Base64 codieren des anlages")
  },
  {//MESS_INTERN_SOCK
    _T("Interne fout: Connectie verloren door programmeer fout!")
   ,_T("Interal error: Connection lost due to programming error!")
   ,_T("Faute interne: Perdue la connection per un erreur programmatique!")
   ,_T("Internes fehler: Verbindung verlohren dürch eine programmatische fehler!")
  },
  {//MESS_FAIL_EHLO
    _T("Een onverwachte fout bij het zenden van het 'EHLO' (Extended-HALO) commando")
   ,_T("An unexpected error occurred while sending the EHLO (Extended-HALO) command")
   ,_T("Echouer de envoyer la commande 'EHLO' (Extended-HALO)")
   ,_T("Nicht erwartettes fehler bei das senden des 'EHLO' kommando (Extended-HALO)")
  },
  {//MESS_UNEX_EHLO
    _T("Fout bij het zenden van het EHLO (Extended-HALO) bericht, we proberen het simpele HALO")
   ,_T("An unexpected EHLO (Extended HALO) response was received, trying simple HELO")
   ,_T("Faute en la commande EHLO (Extended HALO): Essayer la simple HELO")
   ,_T("Fehler en das kommando EHLO (Extended HALO): Wir probieren das simpeler HELO")
  },
  {//MESS_FAIL_HELO
    _T("Een onverwachte fout bij het verzenden van het 'HELO' commando")
   ,_T("An unexpected error occurred while sending the 'HELO' command")
   ,_T("Echouer de envoyer la commande 'HELO'")
   ,_T("Nicht erwartettes fehler bei das senden des 'HELO' kommando")
  },
  {//MESS_UNEX_HELO
    _T("Fout bij het inloggen. Onverwacht antwoord op het 'HELO' commando")
   ,_T("Error at logon. An unexpected HELO response was received")
   ,_T("Faute a la logg-on. Reçu une response n'attende pas a la commande 'HELO'")
   ,_T("Fehler bei das anloggen. Nicht erwartette anwort auf das 'HELO' kommando entfangen")
  },
  {//MESS_IMPORTANCE
      _T("Prioriteit")
     ,_T("Importance")
     ,_T("Priorité")
     ,_T("Priorität")
  },
  {//MESS_HIGH
      _T("Hoog")
     ,_T("High")
     ,_T("Haute")
     ,_T("Hoch")
  },
  {//MESS_NORMAL
      _T("Normaal")
     ,_T("Normal")
     ,_T("Normale")
     ,_T("Normal")
  },
  {//MESS_LOW
      _T("Laag")
     ,_T("Low")
     ,_T("Basse")
     ,_T("Niedrig")
  },
  {// MESS_DELIVERY
      _T("Melding ontvangst indien")
     ,_T("Delivery notification when")
     ,_T("Notification de remettere")
     ,_T("Lieferungsbevestigung als")
  },
  {//MESS_DELIVERY_FAILED
      _T("Mislukt")
     ,_T("Failed")
     ,_T("Manque")
     ,_T("Verfehlen")
  },
  {//MESS_DELIVERY_NOTIFY
      _T("Bezorgd")
     ,_T("Delivered")
     ,_T("Remettre")
     ,_T("Besorgt")
  },
  {//MESS_DELIVERY_DELAY
      _T("Vertraagd")
     ,_T("Delay")
     ,_T("Ralentir")
     ,_T("Verlangsamt")
  },
  {//MESS_DELIVERY_NEVER
      _T("Nooit")
     ,_T("Never")
     ,_T("Jamais")
     ,_T("Niemals")
  },
  {//MESS_DELIVERY_HEADERS
      _T("Koptekst")
     ,_T("Headers")
     ,_T("En-tête")
     ,_T("Kopftekste")
  },
  {//MESS_DELIVERY_FULL
      _T("Volledig")
     ,_T("Full")
     ,_T("Complet")
     ,_T("Völlig")
  },
  {//MESS_DISPOSITION
      _T("Leesbevestiging")
     ,_T("Read disposition")
     ,_T("Affirmation de lire")
     ,_T("Liese befestigung")
  },
  {//MESS_DISP_YES
      _T("Ja")
     ,_T("Yes")
     ,_T("Oui")
     ,_T("Ja")
  },
  {//MESS_DISP_NO
      _T("Nee")
     ,_T("No")
     ,_T("Non")
     ,_T("Nein")
  },
  { // MESS_FAIL_AUTH
      _T("Fout bij aanvragen van authenticatie")
     ,_T("Failure at the LOGON authentication request")
     ,_T("Faute en envoyer la authentication")
     ,_T("Fehler bei anfragen des authentications")
  },
  {// MESS_UNEX_AUTH
      _T("Fout bij login command: Onverwachte respons van de server")
     ,_T("Unexpected answer from server at login command")
     ,_T("Response n'a pas expecte a la LOGON")
     ,_T("Fehler bei das authenticieren")
  },
  {// MESS_FAIL_MAILID
      _T("Fout bij het versturen van het mail-id (username)")
     ,_T("Failed to send the mailid (username) for logon")
     ,_T("Faute en envoyer la identification e-mail (mailid)")
     ,_T("Fehler bei das senden der identität (mailid)")
  },
  {// MESS_UNEX_MAILID
      _T("Onverwacht antwoord na het versturen van het mail-id (username)")
     ,_T("Unexpected answer after sending the mailid (username)")
     ,_T("Reçu une response n'attendee pas d'apres envoyer la identification (mailid)")
     ,_T("Fehler nach dem senden der identität (mailid)")
  },
  { // MESS_NO_MAILID
      _T("Geen mailid doorgegeven aan de server of een onacceptable mailid doorgegeven")
     ,_T("No mail-id at logon time givven or unacceptable mail-id for the server")
     ,_T("N'est envoyer pas une identité (ou une identité n'a pas acceptable)")
     ,_T("Keine mail identität versonden, oder nicht acceptierte mail identität")
  },
  {// MESS_FAIL_PASSWORD
      _T("Fout bij het versturen van het (ge-encrypte) wachtwoord")
     ,_T("Error at the sending of the (encrypted) password")
     ,_T("Faute an envoyer la (encrypte) mot de passe")
     ,_T("Fehler bei senden des passwort")
  },
  {// MESS_UNEX_PASSWORD
      _T("Onverwacht antwoord na het versturen van het (ge-encrypte) wachtwoord")
     ,_T("Unexpected answer after sending the (encrypted) password")
     ,_T("Reçu une response n'expecte pas devant l'envoyer de la mot de passe")
     ,_T("Nicht erwartette antwort nach senden des passwort")
  },
  {// MESS_LOGIN_IDPWD
      _T("Vereist login: maar geen mail-id en wachwoord opgegeven")
     ,_T("Requested login: no mail-id or password givven")
     ,_T("Exige d'une identification: pas de identité (mailid) ou mot de passe")
     ,_T("Erforderte identität: keine identität (mailid) oder passwort")
  },
  {// MESS_BUTTON_ADD
      _T("Adressen")
     ,_T("Addresses")
     ,_T("Addresee")
     ,_T("Adresierten")
  },
  {// MESS_BUTTON_PROFILE
      _T("Profiel")
     ,_T("Profile")
     ,_T("Profil")
     ,_T("Profil")
  },
  {// MESS_BUTTON_CONFIG
      _T("Instelling")
     ,_T("Config")
     ,_T("Profil")
     ,_T("Configuration")
  },
  {// MESS_PROFILE_DOUBLE
      _T("Profielnaam komt al voor: ")
     ,_T("Profilename is not unique: ")
     ,_T("Profil pas unique: ")
     ,_T("Configuration besteht: ")
  },
  {// MESS_PROFILE_REMOVE
      _T("Wilt u het profiel '%s' verwijderen?")
     ,_T("Would you like to remove the profile '%s'?")
     ,_T("Supprimer la configuration '%s'?")
     ,_T("Der Konfiguration '%s' entfernen?")
  },
  {// MESS_PROFILE_EDIT
      _T("Wilt u het profiel '%s' wijzigen?")
     ,_T("Would you like to edit the profile '%s'?")
     ,_T("Modifier la configuration '%s'?")
     ,_T("Der Konfiguration '%s' ändern?")
  },
  {// MESS_PASSWORDS
      _T("De wachtwoorden zijn niet aan elkaar gelijk. Corrigeer de wachtwoorden eerst!")
     ,_T("The passwords are not the same. Correct your passwords before saving!")
     ,_T("Les deux mot de passe ne sont pas égal. Corriger les mots avant préserver la fiche!")
     ,_T("Beide Kenwörter sind nicht gleich. Korrigiere Ihre Kenwörter bitte!")
  },
  {// MESS_NO_PROFILES
      _T("U heeft nog geen profielen gedefinieerd!\n\n")
      _T("Gebruik de knop 'Nieuw' om een of meerdere mailprofielen aan te maken\n")
      _T("en kies daarna op dit venster uw mailprofiel voor het verzenden.")
     ,_T("You have no mail profiles defined as of yet!\n\n")
      _T("Use the 'New' button to create one or more mail profiles\n")
      _T("and choose from them on this dialog upon return.")
     ,_T("Vous n'avez pas creer des configuration des emails!\n\n")
      _T("Utiliser 'Nouveau' et creer une ou plus des configuration des email\n")
      _T("et choisi d'avant les configurations en retourner en la fiche de profiles.")
     ,_T("Sie haben noch keine email-profielen definiert!\n\n")
      _T("Brauche den 'Neu' Taste um eins oder mehrere profielen zu definieren\n")
      _T("und wahlen Sie von diesen when Sie hier zuruck kommen.")
  },
  {// MESS_ADRES_DELETE
      _T("Wilt u de geadresseerde '%s' verwijderen uit de lijst?")
     ,_T("Would you like remove the email address '%s'?")
     ,_T("Supprimer la addresse email '%s'?")
     ,_T("Wollen sie das email adres '%s' entfernen?")
  },
  {// MESS_ADRES_SPECIALS
      _T("Email adressen mogen geen speciale tekens bevatten:(),:;<>@[\\]")
     ,_T("Email addresses cannot contain special characters: (),:;<>@[\\]")
     ,_T("Les characters espaciale sont interdit dans les adresses 'd email: (),:;<>@[\\]")
     ,_T("Email adressen können keine spezial zeigen haben: (),:;<>@[\\]")
  },
  {// MESS_NO_AUTHOR
      _T("U bent niet geautoriseerd voor deze mail-server via deze gebruiker/wachtwoord combinatie!")
     ,_T("You are not authorised for this mail-server with this user/password combination!")
     ,_T("Vous n'avez pas authorisee avec cet combinaison de code 'd utilisateur/mot de passe!")
     ,_T("Sie sind nicht authorisiert mit dieses kombination von Benutzer/Kennwort")
  },
  {// MESS_NO_LOGIN
      _T("Geen gebruikersnaam en wachtwoord opgegeven. Probeer mail te versturen zonder login!")
     ,_T("No user-ID and password given: Trying to relay mail without login!")
     ,_T("Pas de code 'd utilisateur/mot de passe. Essayer envoyer la e-mail sans login!")
     ,_T("Kein Benutzer/Kennwort bekant. Probiere email zu senden ohne login")
  },
  {// MESS_SINGLE_SIGNON
      _T("Probeer in te loggen met MS-Windows single signon zonder gebruiker/wachtwoord.")
     ,_T("Try MS-Windows single signon with no username/password.")
     ,_T("Esssayer une unique code d'accès sans utilisateur/mot de passe.")
     ,_T("Probiert werd um single signon zu betrachten ohne Benutzer/Kennwort.")
  },
  { // MESS_INST_DEFAULTS
      _T("Kan geen standaard profielen bestand schrijven in de installatie directory van PostMail")
     ,_T("No standard profiles written in the installation directory of PostMail")
     ,_T("Aucun fichier de profils-standards écriture dans le répertoire d'installation courrier postal PostMail")
     ,_T("Kein Standard-Profile-Datei geschrieben im Installationsverzeichnis des PostMail")
  },
  {// MESS_NOT_WRITTEN
      _T("Kan mail bestand niet naar de mailcache wegschrijven")
    , _T("Cannot write the email back to the email cache")
    , _T("Pas possible de ecrire le email a la caché")
    , _T("Nicht möglich das email nach dem cache zu schreiben")
  },
  {// MESS_NOPROFILE
      _T("Het is niet mogelijk om een profiel te kiezen, omdat er nog geen profielen zijn.\nMaak er eerst een profiel aan met de knop 'Instelling'")
     ,_T("It's not possible to choose a profile, because their are none defined.\nCreate a new profile first with the button 'Config'")
     ,_T("Il ne est pas possible de sélectionner un profil, car il n'y a pas de profils. Faire d'abord un profil avec le bouton 'Profile'")
     ,_T("Es ist nicht möglich, ein Profil zu wählen, weil es keine Profile gibt. Stellen Sie zunächst ein Profil zusammen mit dem 'Profil' Schaltfläche")
  },
  {// MESS_ADDATTACHMENT
      _T("Selecteer een bijlage bij de email")
     ,_T("Select an attachment for this email")
     ,_T("Selecté une annexe por cet email")
     ,_T("Selecteren Sie eine Anlage")
  },
  {// MESS_REMOVEATTACHMENT
      _T("Wilt u de bijlage [%s] verwijderen?")
     ,_T("Do you want to remove attachment [%s] ?")
     ,_T("Peut tu destruire la annexe [%s]?")
     ,_T("Wollen Sie die Anlage [%s] entfernen?")
  },
  {// MESS_BLOCKED_ATTACH
      _T("In verband met de veiligheid is deze bijlage geblokkeerd voor openen")
     ,_T("As a safety precaution, this attachment is blocked for opening ")
     ,_T("Mesure de sécurité: Cet annexe est bloque pour l'ouverture")
     ,_T("Aus Sicherheitsgründen ist diese Anlage für Öffnen gesperrt")
  },
  {// MESS_INETTYPE
      _T("Kan het type internet verbinding niet bepalen (IP4 of IP6)")
     ,_T("Cannot determine if internet is of type IP4 or IP6")
     ,_T("Pas possible de determinee une connection de IP4 ou de IP6")
     ,_T("Es is nicht möglich die IP4 oder die IP6 verbinding zu wahlen")
  },
  {// MESS_BUTTON_ADDATTACH
      _T("Voeg toe [...]")
     ,_T("Add [...]")
     ,_T("Ajouter [...]")
     ,_T("Anfügen [...]") 
  },
  {// MESS_BUTTON_DELATTACH
      _T("Verwijder")
     ,_T("Remove")
     ,_T("Retirer")
     ,_T("Entfernen") 
  },
  {// MESS_BUTTON_SHOWATTACH
      _T("Toon")
     ,_T("Show")
     ,_T("Expose")
     ,_T("Zeigen")
  },
  {// MESS_INST_SERVERS
      _T("Geen standaard email servers gevonden in 'Postmail.Mailservers.ini' in de installatie directory van PostMail")
     ,_T("No default e-mail servers found in 'Postmail.Mailservers.ini' in the installation directory PostMail")
     ,_T("Aucun serveur de messagerie par défaut trouvés dans 'Postmail.Mailservers.ini' dans le répertoire d'installation du PostMail")
     ,_T("Keine Standard-E-Mail-Server in 'PostMail.Mailservers.ini' im Installationsverzeichnis von Postmail gefunden")
  },
  {// MESS_ATTACHMENT
      _T("Bijlage")
     ,_T("Attachment")
     ,_T("Annexe")
     ,_T("Datei")
  },
  {// MESS_SIZE_TEXT
      _T("Grootte")
     ,_T("Size")
     ,_T("Dimension")
     ,_T("Grösse")
  },
  {// MESS_BUTTON_BOLD
      _T("Vet")
     ,_T("Bold")
     ,_T("Gros")
     ,_T("Fett")
  },
  {// MESS_BUTTON_ITALIC
      _T("Schuin")
     ,_T("Italic")
     ,_T("Italique")
     ,_T("Kursiv")
  },
  {// MESS_BUTTON_UNDERLINE
      _T("Onderstreep")
     ,_T("Underline")
     ,_T("Souligner")
     ,_T("Unterstreich")
  },
  {// MESS_BUTTON_STRIKE
      _T("Doorhalen")
     ,_T("Strikethrough")
     ,_T("Grève")
     ,_T("Streiken")
  },
  {// MESS_BUTTON_FONT
      _T("Lettertype [...]")
     ,_T("Font [...]")
     ,_T("Font face [...]")
     ,_T("Schriftart [...]")
  },
  {// MESS_BUTTON_COLOR
      _T("Kleur [...]")
     ,_T("Color [...]")
     ,_T("Couleur [...]")
     ,_T("Farbe [...]")
  },
  {// MESS_MAX_OUTBOX
      _T("Uw outbox map bevat nu [%d] berichten, terwijl het maximum [%d] is. Ruim uw berichten op!")
     ,_T("Your outbox contains [%d] emails, while the maximum is [%d]. Clean out your outbox!")
     ,_T("Votre boîte d'envoi contient [%d] e-mails, alors que le maximum est de [%d]")
     ,_T("Ihre outbox enthält [%d] E-Mails, während die maximale number ist [%d]")
  },
  {//   
      _T("Versie informatie in het programma is gecomprommiteerd. Het programma wordt afgesloten")
     ,_T("Version information in the program has been compromised. The program will be closed")
     ,_T("Les informations de version dans le programme a-été compromise. Le programme se ferme")
     ,_T("Im Programm enthaltenen version Informationen ist beeinträchtigt. Das Programm wird geschlossen")
  },
  {// MESS_RUNTIME_RENAMED
      _T("Het programma is hernoemd. Dit is niet toegestaan. Het programma wordt afgesloten.")
     ,_T("The programm has been renamed. This is not allowed. The program will be closed")
     ,_T("Le programme a été renommé. Ce n'est pas autorisé. Le programme sera fermé")
     ,_T("Das Programm wurde umbenannt. Dies ist nicht erlaubt. Das Programm wird geschlossen")
  },
  {// MESS_FAIL_TLS
      _T("STARTTLS commando kon niet naar de server verstuurd worden, veilige mode is NIET opgestart.")
     ,_T("STARTTLS commando could not be sent to the server, secure mode not initialized")
     ,_T("Commando STARTTLS pourrait pas être envoyé au serveur, non initialisé en mode sécurité")
     ,_T("STARTTLS-Kommando kann nicht an den Server gesunden werden, die sichere Modus wird nicht initialisiert")
  },
  {// MESS_UNEX_TLS
      _T("Onverwacht antwoord op het STARTTLS commando om veilige mode op te starten.")
     ,_T("Unexpected answer received at the STARTTLS command, no way to start secure mode.")
     ,_T("Réponse inattendue reçue à la commande STARTTLS, aucun moyen de lancer le mode sécurité.")
     ,_T("Unerwartete Antwort erhielt am STARTTLS Befehl, keiner Weise den sicheren Modus zu starten.")
  },
  {// MESS_TEMP_NOTLS
      _T("Secure TLS modus tijdelijk niet beschikbaar. Vervolg in onveilige modus.")
     ,_T("Secure TLS mode temporary unavailable. Continue in unencrypted mode!")
     ,_T("Mode sécurisé TLS temporaire indisponible. Continuer dans le mode non crypté!")
     ,_T("Sichere TLS vorübergehend nicht verfügbar. Weiter in unverschlüsselter Art und Weise")
  },
  {// MESS_TOO_BIG
      _T("Het bericht (inclusief bijlagen) is te groot voor de mailserver")
     ,_T("The messages (including attachments) is too big for the mailserver")
     ,_T("La communique (avec annexes) est trop gros pour la serveur de messagerie")
     ,_T("Die Nahrichte (mit Anlagen) ist zu gross fur die Mail-Server")
  },
  { // MESS_UNEX_AUTHTYPE
      _T("Authenticatie type is niet gesupport. Ik ken alleen: PLAIN, NTLM en XOAUTH2")
     ,_T("Authentication type is not supported. All i know is: PLAIN, NTLM and XOAUTH2")
     ,_T("Sorte de authentication inconnu. Tous que je sais: PLAIN, NTLM et XOAUTH2")
     ,_T("Authentifizierungstyp ist unbekannt. Ich kenne nur: PLAIN, NTLM und XOAUTH2")
  },
  {// MESS_FILE_DELETE
      _T("Kan bestand niet verwijderen: [%d] %s")
     ,_T("Cannot delete the file: [%d] %s")
     ,_T("Pas possible de supprimer la fiche: [%d] %s")
     ,_T("Kan die datei nicht entfernen: [%d] %s")
  },
  {// MESS_ATTACHMENT_IN_USE
     _T("Kan bijlage niet openen. Staat het nog open in een viewer?\nBijlage: ")
    ,_T("Cannot open an attachment. Is it still opened in a viewer?\nAttachment: ")
    ,_T("Impossible d'ouvrir l'accessoire. Est-ce que c'est encore ouvert dans un téléspectateur ??\nAccessoire: ")
    ,_T("Anhang kann nicht geöffnet werden. Ist es in einem Betrachter noch geöffnet?\nAnhang: ")
  },
  {// MESS_DO_NOT_EDIT
     _T("Eventuele wijzigingen in de bijlage zullen verloren gaan.\nWijzig het document voordat u het e-mailt.")
    ,_T("Any changes to the attachment will be lost.\nChange the document before emailing it.")
    ,_T("Toute modification apportée à la pièce jointe sera perdue.\nChanger le document avant de l'envoyer par e-mail.")
    ,_T("Jegliche Änderungen am Anhang gehen verloren.\nÄndern Sie das Dokument, bevor Sie es per E-Mail verschicken.")
  },
  {// MESS_WRITE_DEFAULT_FONT
     _T("Font gegevens ook wegschrijven als 'default font' voor alle gebruikers?")
    ,_T("Write font data as 'default font' for all users?")
    ,_T("Inscrire les données de police comme 'police par défaut' pour tous les utilisateurs?")
    ,_T("Schriftdaten als 'Standardschriftart' für alle Benutzer schreiben?")
  },
  {// MESS_SENDING_TO
     _T("Verzenden naar")
    ,_T("Sending to")
    ,_T("Envoyer à")
    ,_T("Senden an")
  },
  {// MESS_NEW
     _T("Nieuw")
    ,_T("New")
    ,_T("Nouveau")
    ,_T("Neu")
  },
  {// MESS_MODIFY
     _T("Wijzigen")
    ,_T("Modify")
    ,_T("Changement")
    ,_T("Ändern")
  },
  {// MESS_DELETE
     _T("Verwijderen")
    ,_T("Delete")
    ,_T("Supprimer")
    ,_T("Löschen")
  },
  {// MESS_ADDADDRESS
     _T("Nieuw email adres")
    ,_T("New email address")
    ,_T("Nouvelle adresse")
    ,_T("Neue E-Mail-Adresse")
  },
  {// MESS_EMAILADDRESS
     _T("Email adres")
    ,_T("Email address")
    ,_T("Adresse email")
    ,_T("E-Mail adresse")
  },
  {// MESS_FRIENDLYNAME
     _T("Leesbare naam")
    ,_T("Friendly name")
    ,_T("Nom lisible")
    ,_T("Lesbarer name")
  },
  {// MESS_PUTINBCC
     _T("Plaats in de BCC lijst")
    ,_T("Place in the BCC list")
    ,_T("Inscrire dans la liste BCC")
    ,_T("Platz in der BCC-Liste")
  },
  {// MESS_CHOOSEPROFILE
     _T("Er is geen geldig profiel voor het verzenden van email opgegeven; zonder dit profiel is het niet bekend vanaf welke mailbox uw bericht verstuurd moet worden. Kies uit de lijst hieronder het(actieve) profiel dat u wilt gebruiken, zodat uw bericht alsnog verzonden kan worden.")
    ,_T("No valid profile found for the sending of the email; without this profile we do not know from which mailbox we should send your message. Choose from the list below the (active) profile you wish to use, so we can send your message correctly.")
    ,_T("Aucun profil d'envoi de courriel valide n'a été spécifié; sans ce profil, il est impossible de déterminer la boîte aux lettres à partir de laquelle votre message doit être envoyé. Veuillez sélectionner le profil (actif) que vous souhaitez utiliser dans la liste ci-dessous afin que votre message puisse être envoyé.")
    ,_T("Es wurde kein gültiges E-Mail-Absenderprofil angegeben. Ohne dieses Profil ist nicht bekannt, von welchem ??Postfach Ihre Nachricht gesendet werden soll. Wählen Sie unten das gewünschte (aktive) Profil aus, damit Ihre Nachricht versendet werden kann.")
  },
  {// MESS_MANAGEPROFILE 
     _T("Wilt u eerst uw profiel aanpassen of een nieuw profiel aanmaken, dan kunt u dit doen met de 'Beheren' knop.")
    ,_T("Do you want to alter a profile or create a new one? You can do so with the 'Manage' button.")
    ,_T("Si vous souhaitez modifier votre profil ou en créer un nouveau, vous pouvez le faire avec le bouton 'Gérer'.")
    ,_T("Wenn Sie zuerst Ihr Profil bearbeiten oder ein neues Profil erstellen möchten, können Sie dies mit der Schaltfläche 'Verwalten' tun.")
  },
  {// MESS_PROFILELIST
     _T("Overzicht actieve mailprofielen")
    ,_T("List of all active mail profiles")
    ,_T("Aperçu des profils de messagerie actifs")
    ,_T("Übersicht über aktive E-Mail-Profile")
  },
  {// MESS_CHOSENPROFILE
     _T("Gekozen mailprofiel")
    ,_T("Chosen mail profile")
    ,_T("Profil de messagerie sélectionné")
    ,_T("Ausgewähltes E-Mail-Profil")
  },
  {// MESS_MISSINGPROFILE
     _T("Kies afzender profiel")
    ,_T("Choose sender profile")
    ,_T("Sélectionner le profil de l'expéditeur")
    ,_T("Absenderprofil auswählen")
  },
  {// MESS_PROFILE
     _T("Profiel")
    ,_T("Profile")
    ,_T("Profil")
    ,_T("Profil")
  },
  {// MESS_SENDER
     _T("Afzender")
    ,_T("Sender")
    ,_T("Expéditeur")
    ,_T("Absender")
  },
  {// MESS_MAILSERVER
     _T("Mailserver")
    ,_T("Mailserver")
    ,_T("Serveur de messagerie")
    ,_T("Mailserver")
  },
  {// MESS_LOGINNAME
     _T("Login naam")
    ,_T("Login name")
    ,_T("Nom d'utilisateur")
    ,_T("Anmeldename")
  },
  {// MESS_DONT_ASK_AGAIN
     _T("Niet meer vragen tijdens de sessie")
    ,_T("Don't ask again during this session")
    ,_T("Plus de questions pendant la session")
    ,_T("Nicht mehr fragen dieser sitzung")
  },
  {// MESS_ORIGINAL
     _T("Origineel")
    ,_T("Original")
    ,_T("Original")
    ,_T("Original")
  },
  {// MESS_MANAGE
     _T("Beheren")
    ,_T("Manage")
    ,_T("Gérer")
    ,_T("Verwalten")
  },
  {// MESS_MANPROFILES
     _T("Beheer profielen")
    ,_T("Manage profiles")
    ,_T("Gérer les profils")
    ,_T("Profile verwalten")
  },
  {// MESS_UP
     _T("Naar boven")
    ,_T("Promote")
    ,_T("Au sommet")
    ,_T("Nach oben")
  },
  {// MESS_DOWN
     _T("Naar beneden")
    ,_T("Demote")
    ,_T("Ver le bas")
    ,_T("Nach untern")
  },
  {// MESS_PROFILE_NAME
     _T("Profiel naam")
    ,_T("Profile name")
    ,_T("Nom de profil")
    ,_T("Profilname")
  },
  {// MESS_EMAIL_ADDRESS
     _T("E-Mail adres")
    ,_T("E-Mail address")
    ,_T("Adresse email")
    ,_T("E-Mail adresse")
  },
  {// MESS_BCC_ADDRESS
     _T("Verzonden items (via BCC)")
    ,_T("Sent items (via BCC)")
    ,_T("Email envoyés (via BCC)")
    ,_T("Gesendete E-Mail (per BCC)")
  },
  {// MESS_SMTP_SERVER
     _T("SMTP Mailserver")
    ,_T("SMTP-Mailserver")
    ,_T("Serveur de message SMTP")
    ,_T("SMTP-Mailserver")
  },
  {//  MESS_SMTP_PORT
     _T("SMTP port number")
    ,_T("SMTP port number")
    ,_T("Numéro de port SMTP")
    ,_T("SMTP-Portnummer")
  },
  {// MESS_SET_AS_DEFAULT
     _T("Standaard profiel")
    ,_T("Standard profile")
    ,_T("Profil standard")
    ,_T("Standardprofil")
  },
  {// MESS_USE_LOGIN
     _T("Gebruik login")
    ,_T("Use login")
    ,_T("Utiliser la connexion")
    ,_T("Login verwenden")
  },
  {// MESS_USER_LOGIN
     _T("Login gebruikernaam")
    ,_T("Login username")
    ,_T("Identifiant de connexion")
    ,_T("Benutzername für Login")
  },
  {// MESS_PASSWORD
     _T("Wachtwoord")
    ,_T("Password")
    ,_T("Mot de passe")
    ,_T("Passwort")
  },
  {// MESS_PASSWORD_AGAIN
     _T("Wachtwoord controle")
    ,_T("Password check")
    ,_T("Contrôle des mots de passe")
    ,_T("Passwortkontrolle")
  },
  {// MESS_OUTBOX
     _T("Postvak UIT")
    ,_T("Outbox")
    ,_T("Boîte d'envoi")
    ,_T("Postausgang")
  },
  {// MESS_SEARCH
     _T("Zoeken")
    ,_T("Search")
    ,_T("Rechercher")
    ,_T("Suchen")
  },
  {// MESS_OUTBOXVIEWER
     _T("Postvak UIT")
    ,_T("Outbox")
    ,_T("Boîte d'envoi")
    ,_T("Postausgang")
  },
  {// MESS_AGAINSHOW
     _T("Opnieuw / Tonen")
    ,_T("Again / Show")
    ,_T("Encore / Afficher")
    ,_T("Wieder / Zeigen")
  },
  {// MESS_SELECTION
     _T("Selectie")
    ,_T("Selection")
    ,_T("Sélection")
    ,_T("Auswahl")
  },
  {// MESS_LASTDAY
     _T("Laatste 24 uur")
    ,_T("Last 24 hours")
    ,_T("Dernières 24 heures")
    ,_T("Letzte 24 Stunden")
  },
  {// MESS_LASTWEEK 
     _T("Laatste week")
    ,_T("Last week")
    ,_T("Semaine dernière")
    ,_T("Letzte Woche")
  },
  {// MESS_LASTMONTH 
     _T("Laatste maand")
    ,_T("Last month")
    ,_T("Mois dernier")
    ,_T("Letzten Monat")
  },
  {// MESS_LAST3MONTHS 
     _T("Laatste 3 maanden")
    ,_T("Last 3 months")
    ,_T("Les 3 derniers mois")
    ,_T("Letzte 3 Monate")
  },
  {// MESS_LAST6MONTHS 
     _T("Laatste 6 maanden")
    ,_T("Last 6 months")
    ,_T("Les 6 dernier mois")
    ,_T("Letzte 6 Monate")
  },
  {// MESS_LASTYEAR 
     _T("Laatste jaar")
    ,_T("Last year")
    ,_T("L'année dernière")
    ,_T("Letztes Jahr")
  },
  {// MESS_ALLMAIL
     _T("Alle e-mails")
    ,_T("All email")
    ,_T("Tous les e-mails")
    ,_T("Alle E-Mails")
  },
  {// MESS_CONFIGOUTBOX
     _T("Configureer postvak")
    ,_T("Configure outbox")
    ,_T("Configurer")
    ,_T("Konfigurieren")
  },
  {// MESS_LOGOUTBOX
     _T("Configureer logbestand")
    ,_T("Configure logfile")
    ,_T("Configurer le journal")
    ,_T("Protocol Konfigurieren")
  },
  {// MESS_OUTBOXACTIVE
     _T("Centraallog actief")
    ,_T("Central log active")
    ,_T("Journal central actif")
    ,_T("Zentrales Protokoll aktiv")
  },
  {// MESS_DATETIME
     _T("Tijdstip")
    ,_T("Datetime")
    ,_T("Temps")
    ,_T("Zeit")
  },
  {// MESS_RESULT
     _T("Resultaat")
    ,_T("Result")
    ,_T("Résultat")
    ,_T("Ergebnis")
  },
  {// MESS_RECEIVER
     _T("Ontvanger")
    ,_T("Receiver")
    ,_T("Destinataire")
    ,_T("Empfänger")
  },
  {// MESS_BESTAND
     _T("Bestand")
    ,_T("File")
    ,_T("Déposer")
    ,_T("Datei")
  },
  {// MESS_NOBOXCONFIG
     _T("Geen postvak UIT geconfigureerd")
    ,_T("No outbox configured")
    ,_T("Aucune boîte d'envoi configurée")
    ,_T("Kein Postausgang konfiguriert")
  },
  {// MESS_PERSOUTBOX
     _T("Persoonlijke postvak UIT: ")
    ,_T("Users outbox: ")
    ,_T("Boîte d'envoi personnelle: ")
    ,_T("Persönlicher Postausgang: ")
  },
  {// MESS_PRIVATEBOX
     _T("Je eigen prive OUTBOX: ")
    ,_T("Your own private outbox: ")
    ,_T("Votre boîte d'envoi privée: ")
    ,_T("Ihr eigener privater Postausgang: ")
  },
  {// MESS_ORGOUTBOX
     _T("Postvak UIT van de organisatie: ")
    ,_T("Outbox of the organisation: ")
    ,_T("Boîte d'envoi de l'organisation: ")
    ,_T("Postausgang der Organisation: ")
  },
  {// MESS_NOBOXHELP
     _T("<GEEN POSTVAK UIT GECONFIGUREERD> : Gebruik de knop 'Configureer postvak' voor je eigen outbox")
    ,_T("<NO OUTBOX CONFIGURED> : Use the button 'Configure outbox' for your own outbox")
    ,_T("<AUCUNE BOÎTE D'ENVOI CONFIGURÉ> : Utilisez le bouton 'Configurer la boîte d'envoi' pour votre propre boîte d'envoi.")
    ,_T("<KEIN POSTAUSGANG KONFIGURIERT> : Verwenden Sie die Schaltfläche \"Postausgang konfigurieren\", um Ihren eigenen Postausgang zu konfigurieren.")
  },
  {// MESS_DELEMAILS
     _T("U gaat nu [%d] email%s verwijderen. Weet u het zeker ?")
    ,_T("You are about to delete [%d] email%s. Are you sure ?")
    ,_T("Vous allez maintenant supprimer [%d] courriel%s. Êtes-vous sûr ?")
    ,_T("Sie werden nun [%d] E-Mail%s zu löschen. Sind Sie sicher ?")
  },
  {// MESS_PROMPTLOG
     _T("Centraal logbestand voor PostMail")
    ,_T("Central logfile for PostMail")
    ,_T("Fichier journal central pour PostMail")
    ,_T("Zentrale Protokolldatei für PostMail")
  },
  {// MESS_LOGEXTENT
     _T("Logfile voor Postmail *.txt")
    ,_T("Central logfile for Postmail *.txt")
    ,_T("Fichier journal pour Postmail *.txt")
    ,_T("Logdatei für Postmail *.txt")
  },
  {// MESS_LOGHEADER
     _T("Centraal logbestand geopend door Postmail::Postvak UIT\n")
    ,_T("Central logfile opened by Postmail::Outbox\n")
    ,_T("Fichier journal central ouvert pour la boîte d'envoi du courrier postal\n")
    ,_T("Zentrale Protokolldatei für Postausgang geöffnet\n")
  },
  {// MESS_NOACCESS
     _T("Kan niet schrijven naar het centraal logboek: ")
    ,_T("Cannot write to the central logfile: ")
    ,_T("Impossible d'écrire dans le journal central: ")
    ,_T("Fehler beim Schreiben in das zentrale Protokoll: ")
  },
  {// MESS_CLOSELOG
     _T("Het huidige centrale logboek is [%s]\nU gaat dit logboek nu UITSCHAKELEN!\nWilt u verder gaan ?")
    ,_T("The current central logfile is [%s]\nYou are about to CLOSEDOWN the logfile\nDo you want to continue ?")
    ,_T("Le journal central actuel se trouve dans [%s]\nVous allez maintenant DÉSACTIVER ce journal!\nVoulez-vous continuer ?")
    ,_T("Das aktuelle zentrale Protokoll befindet sich unter [%s]\nSie werden dieses Protokoll nun deaktivieren!\nMöchten Sie fortfahren ?")
  },
  {// MESS_CONFIGUREBOX
     _T("Configureer Postvak UIT")
    ,_T("Configure OUTBOX")
    ,_T("Configurer la boîte d'envoi")
    ,_T("Postausgang konfigurieren")
  },
  {// MESS_TYPEBOX
     _T("Type postvak UIT")
    ,_T("Type outbox")
    ,_T("Type de Boîte")
    ,_T("Postausgang")
  },
  {// MESS_MAPMAIL
     _T("Map e-mails")
    ,_T("E-mail directory")
    ,_T("Dossier e-mails")
    ,_T("E-Mail Ordner")
  },
  {// MESS_INITSELECT
     _T("Initiële selectie")
    ,_T("Initial selection")
    ,_T("Sélection initiale")
    ,_T("Erste Auswahl")
  },
  {// MESS_WARNINGAT
     _T("Waarschuwen bij")
    ,_T("Warning at")
    ,_T("Avertir en cas de")
    ,_T("Warnung im Falle von")
  },
  {// MESS_HISTORY
     _T("Historie")
    ,_T("History")
    ,_T("Histoire")
    ,_T("Geschichte")
  },
  {// SEL_NOOUTBOX
     _T("Geen outbox gebruiken")
    ,_T("Do not use an outbox")
    ,_T("N’utilisez pas de boîte d’envoi.")
    ,_T("Verwenden Sie keinen Postausgang.")
  },
  {// SEL_USEAPPDATA
     _T("Gebruik de 'APPDATA\\PostMail\\OutBox' map")
    ,_T("Use the 'APPDATA\\PostMail\\Outbox' directory")
    ,_T("Utilisez le répertoire 'APPDATA\\PostMail\\Outbox'")
    ,_T("Verwenden Sie das Verzeichnis 'APPDATA\\PostMail\\Outbox.")
  },
  {// SEL_USEPRIVATE
     _T("Gebruik je eigen prive map")
    ,_T("Use your own private directory")
    ,_T("Utilisez votre propre répertoire privé")
    ,_T("Nutzen Sie Ihr eigenes privates Verzeichnis")
  },
  {// SEL_USEORG
     _T("Gebruik een gedeelde map van je organisatie")
    ,_T("Use a shared directory from your organization")
    ,_T("Utilisez un répertoire partagé de votre organisation")
    ,_T("Verwenden Sie ein freigegebenes Verzeichnis Ihrer Organisation.")
  },
  {// SEL_EMAILTODAY
     _T("Emails van vandaag")
    ,_T("Emails of today")
    ,_T("Courriels du jour")
    ,_T("Heutige E-Mails")
  },
  {// SEL_EMAILWEEK
     _T("Emails van de laatste 7 dagen")
    ,_T("Emails of the last 7 days")
    ,_T("Courriels des 7 derniers jours")
    ,_T("E-Mails der letzten 7 Tage")
  },
  {// SEL_EMAILMONTH
     _T("Emails van de laatste 31 dagen")
    ,_T("Emails of the last 31 days")
    ,_T("Courriels des 31 derniers jours")
    ,_T("E-Mails der letzten 31 Tage")
  },
  {// SEL_EMAIL3MONTHS
     _T("Emails van de vorige 3 maanden")
    ,_T("Emails of the last 3 months")
    ,_T("Courriels des 3 derniers mois")
    ,_T("E-Mails der letzten 3 Monate")
  },
  {// SEL_EMAIL6MONTHS
     _T("Emails van de vorige 6 maanden")
    ,_T("EMails of the last 6 months")
    ,_T("Courriels des 6 derniers mois")
    ,_T("E-Mails der letzten 6 Monate")
  },
  {// SEL_EMAILYEAR
     _T("Emails van het laatste jaar")
    ,_T("Emails of the last year")
    ,_T("Courriels de l'année dernière")
    ,_T("E-Mails aus dem letzten Jahr")
  },
  {// SEL_EMAILALL
     _T("Alle emails")
    ,_T("All emails")
    ,_T("Tous les e-mails")
    ,_T("Alle E-Mails")
  },
  {// MESS_OUTBOXDIRS
     _T("Postvak UIT mappen")
    ,_T("Outbox directories")
    ,_T("Dossiers de la boîte d'envoi")
    ,_T("Postausgangsordner")
  },
  {// MESS_NODIRECTORY
     _T("De map bestaat niet: ")
    ,_T("The directory does not exist: ")
    ,_T("Le répertoire n'existe pas: ")
    ,_T("Der Ordner existiert nicht: ")
  },
  {// MESS_CHOOSEOUTBOXDIR
     _T("Kies de map van het postvak UIT")
    ,_T("Choose the outbox directory")
    ,_T("Sélectionnez le dossier Boîte d'envoi")
    ,_T("Wählen Sie den Ordner 'Postausgang' aus.")
  },
  {// MESS_GT2000
     _T("Waarde mag niet kleiner zijn dan 2000")
    ,_T("The value cannot be smaller than 2000")
    ,_T("La valeur ne doit pas être inférieure à 2000")
    ,_T("Der Wert darf nicht unter 2000 liegen.")
  },
  {// MESS_ST150000
     _T("Waarde mag niet groter zijn dan 150.000")
    ,_T("The value may not exceed 150.000")
    ,_T("La valeur ne doit pas dépasser 150 000")
    ,_T("Der Wert darf 150.000 nicht überschreiten.")
  },
  {// MESS_DELMAILBOX
     _T("Wilt u de mailbox [%s] vergeten ?")
    ,_T("Do you want to drop the outbox [%s] ?")
    ,_T("Voulez-vous oublier la boîte aux lettres [%s] ?")
    ,_T("Wollen Sie den E-Mail Ordner [%s] vergessen ?")
  },
  {// MESS_NOMANUAL
     _T("De handleiding is niet gevonden: ")
    ,_T("The manual could not be found: ")
    ,_T("Le manuel est introuvable: ")
    ,_T("Das Handbuch wurde nicht gefunden: ")
  },
  {// MESS_MANUAL365
     _T("Lees in de handleiding het hoofdstuk over de Office-365 configuratie!")
    ,_T("Please read the chapter in the manual on the Office-365 configuration!")
    ,_T("Veuillez lire le chapitre sur la configuration d'Office 365 dans le manuel!")
    ,_T("Bitte lesen Sie das Kapitel zur Office 365-Konfiguration im Handbuch!")
  },
  {// MESS_DEFSERVER
     _T("Geef eerst een default mailserver op in 'PostMail.Mailservers.ini' !!")
    ,_T("First specify a default mail server in 'PostMail.Mailservers.ini' !!")
    ,_T("Commencez par spécifier un serveur de messagerie par défaut dans 'PostMail.Mailservers.ini' !!")
    ,_T("Zuerst muss ein Standard-Mailserver angegeben werden in PostMail.Mailservers.ini' !!")
  },
  {// MESS_WRITELOG
     _T("Kan niet naar het centrale logboek schrijven: ")
    ,_T("Cannot write to the central logfile: ")
    ,_T("Impossible d'écrire dans le journal central: ")
    ,_T("Schreiben in das zentrale Protokoll fehlgeschlagen: ")
  },
  {// MESS_CHOOSECOLOR
     _T("Kies een text kleur")
    ,_T("Choose a text color")
    ,_T("Choisissez une couleur pour le texte")
    ,_T("Wählen Sie eine Textfarbe")
  },
  {// MESS_SAVE
     _T("Opslaan")
    ,_T("Save")
    ,_T("Sauvegarder")
    ,_T("Speichern")
  },
  {// MESS_SMTP25
     _T("25 Onveilige standaard SMTP")
    ,_T("25 Standard insecure SMTP")
    ,_T("25 SMTP standard non sécurisé")
    ,_T("25 Standard unsicheres SMTP")
  },
  {// MESS_SMTP587
     _T("587 Veilige standaard SMTP")
    ,_T("587 Standard secure SMTP")
    ,_T("587 SMTP Sécurisé standard")
    ,_T("587 Standard sicheres SMTP")
  },
  {// MESS_PROFSSAVED
     _T("Lokaal profielenbestand: %s\nOpgeslagen profiel(en): %d")
    ,_T("Local profiles file: %s\nSaved number of profiles: %d")
    ,_T("Fichier de profil local: %s\nProfils enregistrés: %d")
    ,_T("Lokale Profildatei: %s\nGespeicherte Profile: %d")
  },
  {// MESS_PROFSAVEFAIL
     _T("Lokaal profielen bestand [%s] niet opgeslagen!")
    ,_T("Local profile file [%s] not saved!")
    ,_T("Le fichier de profils locaux [%s] n'a pas été enregistré!")
    ,_T("Lokale Profildatei [%s] nicht gespeichert!")
  },
  {// MESS_WINDIRFAIL
     _T("Kan geen directory map maken. Mapnaam: [%s]\nMS-Windows OS Fout: %d")
    ,_T("Cannot create directory. Directory name: [%s]\nMS-Windows OS Error: %d")
    ,_T("Impossible de créer le répertoire. Nom du dossier: [%s]\nErreur système MS-Windows: %d")
    ,_T("Verzeichnis konnte nicht erstellt werden. Ordnername: [%s]\nMS-Windows - Betriebssystemfehler: %d")
  },
  {// MESS_USE1SERVER
     _T("Geen server gevonden. Gebruikt nu de eerste SMTP server: %s")
    ,_T("No server set yet. Using first SMTP server: %s")
    ,_T("Il n'y a pas de serveur utilisé. Utiliser le premier serveur SMTP: %s")
    ,_T("Kein Server verfügbar. Der erste SMTP-Server wird verwendet: %s")
  },
  {// MESS_SERVERNOCONF
     _T("SMTP Server [%s] is ***NIET*** in de lijst van geconfigureerde servers")
    ,_T("SMTP Server [%s] is ***NOT*** in the list of configured servers")
    ,_T("Le serveur SMTP [%s] n'est ***PAS*** dans la liste des serveurs configurés")
    ,_T("Der SMTP-Server [%s] ist ***NICHT*** in der Liste der konfigurierten Server")
  },
  {// MESS_INITRELAY
     _T("Concept e-mail samenstellen")
    ,_T("Create a concept e-mail")
    ,_T("Rédiger un brouillon d'e-mail")
    ,_T("Einen E-Mail-Entwurf verfassen")
  },
  {// MESS_VIEWER
     _T(" VIEWER ")
    ,_T(" VIEWER ")
    ,_T(" VISEUR ")
    ,_T(" BESCHAUWER ")
  },
  {// MESS_MAILFROM
     _T(" - Mailen namens: ")
    ,_T(" - Mailing from: ")
    ,_T(" - Courrier de: ")
    ,_T(" - Absender: ")
  },
  {// MESS_COMMANDLINE
     _T("Opdrachtregel ...")
    ,_T("Command line ...")
    ,_T("Ligne de commande ...")
    ,_T("Befehlszeile")
  },
  {// MESS_MANUAL
     _T("Handboek ..")
    ,_T("Manual ...")
    ,_T("Manuel ...")
    ,_T("Handbuch ...")
  },
  {// MESS_ABOUTBOX
     _T("Over PostMail ...")
    ,_T("About PostMail ...")
    ,_T("À propos de PostMail ...")
    ,_T("Über PostMail ...")
  },
  {// MESS_ASKDELPROFILE
     _T("Het gekozen profiel wordt verwijderd en de oorspronkelijke gegevens,\nzoals meegegeven door het opstartende programma worden opnieuw ingelezen.\n\nWilt u doorgaan?")
    ,_T("The chosen profile will be removed and all original data\nas given by the starting program will be re-read.\n\nDo you want to continue?")
    ,_T("Le profil sélectionné sera supprimé et les données d'origine,\ntelles que fournies par le programme de démarrage, seront restaurées.\n\nVoulez-vous continuer?")
    ,_T("Das ausgewählte Profil wird gelöscht und die Originaldaten,\nwie sie vom Startprogramm bereitgestellt wurden, werden wiederhergestellt.\n\n Möchten Sie fortfahren?")
  },
  {// END OF ARRAY MARKER
     NULL
    ,NULL
    ,NULL
    ,NULL
  }
};
