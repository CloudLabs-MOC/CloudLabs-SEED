# Clickjacking

```
Copyright © 2021 by Collin Howland, Faith Letzkus, Julio Trujillo, and Steve Cole at Washington University
in St. Louis.
This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
License. If you remix, transform, or build upon the material, this copyright notice must be left intact, or
reproduced in a way that is reasonable to the medium in which the work is being re-published.
```
## 1 Overview

Clickjacking, also known as a “UI redress attack,” is an attack that tricks a user into clicking on something
they do not intend to when visiting a webpage, thus “hijacking” the click. In this lab, we will explore a
common attack vector for clickjacking: the attacker creates a webpage that loads the content of a legitimate
page but overlays one or more of its buttons with invisible button(s) that trigger malicious actions. When a
user attempts to click on the legitimate page’s buttons, the browser registers a click on the invisible button
instead, triggering the malicious action.

**Example scenario.** Suppose an attacker acquires the domain _starbux.com_ and creates a website with 
that URL. The site first loads the legitimate target website _starbucks.com_ in an iframe element span- 
ning the entire webpage, so that the malicious _starbux.com_ website looks identical to the legitimate 
_starbucks.com_ website. The attacker’s site then places an invisible button on top of the ‘Menu’ but- 
ton on the displayed _starbucks_ page; the button triggers a 1-click purchase of the attacker’s product on 
Amazon. If the user is logged on to Amazon when they try to click the legitimate button, the inadvertent 
click on the invisible button will make the unintended purchase without the user’s knowledge or consent. 

**Topic coverage.** This lab covers the following topics:

- Clickjacking attack
- Countermeasures: frame busting and HTTP headers
- Iframes and sandboxes
- JavaScript

**Lab environment.** You can perform the lab exercise on the SEED VM provided by the Cloudlabs.

## 2 Lab Environment Setup

In this lab, we will use two websites. The first is the vulnerable homepage of the fictional business “Alice’s Cupcakes”, accessible at _http://www.cjlab.com_. 
The second is the attacker’s malicious web site that is used for hijacking clicks intended for the Alice’s Cupcakes page, accessible at _http://www.cjlab-attacker.com_. 
We use containers to set up the web servers.


### 2.1 Container Setup and Commands

Files needed for this lab are included in Labsetup.zip, which can be fetched by running the following commands.

```
sudo wget https://github.com/CloudLabs-MOC/CloudLabs-SEED/raw/main/Web%20Security/Clickjacking%20Attack/Lab%20files/Labsetup.zip
```

```
sudo unzip Labsetup.zip
```
Enter the Labsetup folder, and use thedocker-compose.ymlfile to set up the lab environment. Detailed explanation of the
content in this file and all the involved Dockerfile can be found from the user manual, which is linked
to the website of this lab. If this is the first time you set up a SEED lab environment using containers, it is
very important that you read the user manual.
In the following, we list some of the commonly used commands related to Docker and Compose. Since
we are going to use these commands very frequently, we have created aliases for them in the.bashrcfile
(in our provided SEEDUbuntu 20.04 VM).
```
$ docker-compose build # Build the container image
$ docker-compose up # Start the container
$ docker-compose down # Shut down the container

// Aliases for the Compose commands above
$ dcbuild # Alias for: docker-compose build
$ dcup # Alias for: docker-compose up
$ dcdown # Alias for: docker-compose down
```
All the containers will be running in the background. To run commands on a container, we often need
to get a shell on that container. We first need to use the _"docker ps"_ command to find out the ID of
the container, and then use _"docker exec"_ to start a shell on that container. We have created aliases for
them in the _.bashrc_ file.
```
$ dockps // Alias for: docker ps --format "{{.ID}} {{.Names}}"
$ docksh <id> // Alias for: docker exec -it <id> /bin/bash

// The following example shows how to get a shell inside hostC
$ dockps
b1004832e275 hostA-10.9.0.5
0af4ea7a3e2e hostB-10.9.0.6
9652715c8e0a hostC-10.9.0.7

$ docksh 96
root@9652715c8e0a:/#

// Note: If a docker command requires a container ID, you do not need to
// type the entire ID string. Typing the first few characters will
// be sufficient, as long as they are unique among all the containers.
```
If you encounter problems when setting up the lab environment, please read the “Common Problems”
section of the manual for potential solutions.

### 2.2 Websites

We use two containers, one running the website for Alice’s Cupcakes (the “defender”, with IP address
_10.9.0.5_) , and the other running the website for the attacker (with IP address _10.9.0.105_). The
IP addresses for these two containers must be consistent in the _docker-compose.yml_ file and the
_/etc/hosts_ file (see below), and we recommend not changing them from their default values.


**The Defender container.** The website for Alice’s Cupcakes is hosted on an Apache web server. The web-
site setup is included in _apachedefender.conf_ inside the defender image folder. The configuration
specifies the URL for the website and the folder where the web application code is stored. The configuration
also contains placeholders for HTTP response headers returned by the server (commented out), which will
be filled in during the course of the lab as a defense countermeasure.

```
<VirtualHost *:80>
      DocumentRoot /var/www/defender
      ServerName www.cjlab.com 
#     Header set <Header-name> "<value>";
#     Header set Content-Security-Policy " \
#             <directive> ’<value>’; \
#           "
</VirtualHost>
```
Because we need to modify the defender’s web page inside this container, for convenience as well as to
allow the modified files to persist beyond the containers, we have mounted a folder (_Labsetup/defender_
on the hosting VM) to the container’s _/var/www/defenderfolder_, which is the _DocumentRoot_ folder
in our Apache configuration. Therefore any files we modify inside the _defender_ folder on the VM will
be seen as modified by the defender’s web server on the container.

**The Attacker container.** The attacker’s website is also hosted on an Apache web server. The website
setup is included in _apacheattacker.conf_ inside the attacker image folder. The Apache configuration
for this website is as follows:
```
<VirtualHost *:80>
    ServerName www.cjlab-attacker.com 
    DocumentRoot "/var/www/attacker"
    DirectoryIndex attacker.html
</VirtualHost>
```
Because we need to modify the attacker’s web page inside this container, for convenience as well as to allow the modified files to persist beyond the containers, we have mounted a folder (_Labsetup/attacker_ on the hosting VM) to the container’s _/var/www/attacker_ folder, which is the _DocumentRoot_ folder 
in our Apache configuration. Therefore any files we modify inside the _attacker_ folder on the VM will 
be seen as modified by the attacker’s web server on the container. 

**Important note.** Any time you make updates to the websites, you may need to clear the browser’s cache
and/or rebuild and restart the containers for the change to be visible, depending on the scope of the change.

**DNS configuration.** We access the defender website and the attacker website using their respective URLs.
To enable these hostnames to be mapped to their corresponding IP addresses, we need to add the following
entries to the _/etc/hostsfile_ on the VM. You need to use the root privilege to change this file (using
_sudo_).
```
10.9.0.5 www.cjlab-attacker.com 
10.9.0.105 www.cjlab-attacker.com 
```
**Test: defender.** Build and start the containers, then navigate to the page _http://www.cjlab.comon_
the VM. You should see a page for Alice’s Cupcakes, a fictional local bakery. If the whole site does not fit in


your window, use _Ctrl + (minus key)_ and _Ctrl + (plus key)_ to zoom out and in respectively
until the site fits. Note that the header and footer buttons on the site are just placeholders and do not contain
live links.

**Test: attacker.** Next, navigate to the page _http://www.cjlab-attacker.com_. You should see a
single button which, when clicked, takes you to a page that tells you you’ve been hacked. In a real attack,
this button could perform a variety of malicious actions.
The goal of the attacker is to overlay this button onto a view of the defender’s webpage displayed on the
attacker’s site, so that a victim user will inadvertently click the malicious button when they think they are
clicking a button on the defender’s webpage.

## 3 Lab Tasks

### 3.1 Task 1: Copy that site!

In the _defender_ folder, you will find the files comprising the website for Alice’s Cupcakes: _index.html _
and _defender.css_. In the _attacker_ folder, you will find the files comprising the attacker’s web- 
site: _attacker.html_ and _attacker.css_. You will be making changes to all of these files except 
_defender.css_ throughout the lab. 

Our first step as the attacker is to add code to _attacker.html_ so that it mimics the Alice’s Cupcakes 
website as closely as possible. A common way to do this is with an HTML Inline Frame element (“iframe”). An iframe enables embedding one HTML page within another. The src attribute of the iframe specifies the site to be embedded, and when the iframe code is executed on a page, the embedded site is loaded into the iframe. 

**Embed the defender’s site into the attacker’s site.**

- Add an iframe HTML element in _attacker.html_ that pulls from _http://www.cjlab.com_.
- Modify the CSS in _attacker.css_ using the _height_, _width_, and _position_ attributes to make
    the iframe cover the whole page and the button overlay the iframe.
- Hints:
    - Explicitly set the iframe to have no border.
    - Investigate the ‘absolute’ and ‘relative’ settings of the _position_ attribute to determine which
       should be used.
- Test your changes by navigating to the attacker’s website. (Remember that you may need to clear the
    browser’s cache and reload the page to see changes made after the initial load.)

**Question:**

1. With the iframe inserted, what does the attacker’s website look like?


### 3.2 Task 2: Let’s Get Clickjacking!

**Basic clickjacking attack.** Add code to the CSS specification of a “button” object given in _attacker.css_
to make the malicious button in _attacker.html_ invisible. Position the button so that it covers the “Ex-
plore Menu” button within the iframe added in the previous Task. There are a variety of ways to accom-
plish this Task; we recommend using the CSS attributes _margin-left_, _margin-top_, _color_, and
_background-color_. When this Task is complete, you will have a functioning clickjacking attack.

**Questions:**

2. How does the appearance of the attacker’s site compare to that of the defender’s site?
3. What happens when you click on the “Explore Menu” button on the attacker’s site?
4. Describe an attack scenario in which the style of clickjacking implemented for this Task leads to
    undesirable consequences for a victim user.

### 3.3 Task 3: Bust That Frame!

“Frame busting” is the practice of preventing a web page from being displayed within a frame, thus de-
fending against the type of attack implemented in the previous Task. One way to bust frames is to include
script code in the webpage source that prevents the site from being framed – that is, it prevents other sites
from opening the webpage in an iframe. In this Task we will add script code to the defender’s webpage that
ensures it is the topmost window on any page where it is being displayed, thus preventing buttons on an
attacker’s page from being overlaid on top of it.

**Write the frame-busting script.** Open the file _defender/index.html_, which contains code for the
Alice’s Cupcakes homepage. We would like to protect the homepage from clickjacking. Your task is to
fill in the Javascript method called _makeThisFrameOnTop()_. Your code should compare _window.top_
and _window.self_ to find out if the top window and the current site’s window are the same object (as
they should be). If not, use the [Location Object](https://developer.mozilla.org/en-US/docs/Web/API/Location) to set the location of the top window to be the same as the
location of the current site’s window. This should be a simple method and take no more than a few lines of
code. Test it out and confirm that your script successfully stops the clickjacker.

**Reminder.** Remember that any time you make changes to one of the websites, you may need to clear the
browser’s cache and reload the page for the changes to take effect.

**Questions:**

5. What happens when you navigate to the attacker’s site now?
6. What happens when you click the button?

### 3.4 Task 4: Attacker Countermeasure (Bust the Buster)

**Disable the frame-busting script.** Now let’s explore how an attacker can create a workaround for front-
end clickjacking defenses like frame busting. There are multiple workarounds, but one of the simplest in the
current scenario is to add the _sandbox_ attribute to the malicious iframe. Read more about the _sandbox_
attribute on [this page](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/iframe) about iframes, then add the _sandbox_ attribute to the iframe in _attacker.html_
and answer the following questions.


**Questions:**

7. What does thesandboxattribute do? Why does this prevent the frame buster from working?
8. What happens when you navigate to the attacker’s site after updating the iframe to use thesandbox
    attribute?
9. What happens when you click the button on the attacker’s site?

### 3.5 Task 5: The Ultimate Bust

As we saw in the previous Task, front-end defenses such as frame busting can be directly circumvented by other front-end settings on the attacker’s webpage and are therefore not sufficient to prevent clickjacking. To prevent clickjacking attacks more comprehensively, we need to set up back-end (i.e. server-side) defenses. Modern websites can cooperate with common browsers to provide such defenses. 

The front-end attacks presented in previous Tasks all rely on the ability of an attacker’s webpage code 
(running in a victim user’s browser) to fetch a benign website’s content before the benign webpage code has a chance to execute any front-end defenses. 

To block this capability, special HTTP headers have been created that specify to browsers the circum- 
stances under which a website’s content should or should not be loaded. By providing one of these HTTP headers with its response to a request for content, the website can instruct browsers to only display the content according to specific matching rules designed to exclude clickjacking attack scenarios. These headers are not part of the official HTTP specification, but are processed by many common browsers. 

You can read more about the XFO attribute [here](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/X-Frame-Options), and more about CSPs [here](https://developer.mozilla.org/en-US/docs/Web/HTTP/CSP). 

**Modify the defender’s response headers.** Open the Apache configuration file for the defender’s website
(_apachedefender.conf_ in the defender’s image folder). Uncomment the lines that specify the HTTP
response headers served with the page, and substitute appropriate text in order to prevent the clickjacking
attack. Specifically, set the X-Frame-Options (XFO) header to the value "_DENY_" and the Content-Security-
Policy (CSP) header to contain the directive "_frame-ancestors ‘none’ _".

**Hint.** Because you are making a change to the server’s configuration, you will need to rebuild and restart
the containers for the change to take effect.

**Questions:**

10. What is the X-Frame-Options HTTP header attribute, and why is it set to “DENY” to prevent the
    attack?
11. What is the Content-Security-Policy header attribute, and why is it set to “frame-ancestors ‘none’ ” to
    prevent the attack?
12. What happens when you navigate to the attacker’s site after modifying each response header (one at a
    time)? What do you see when you click the button?


Learn more. There are other ways to perform clickjacking besides the one explored in this lab, and many
possible malicious consequences beyond the ones suggested here. To learn more about clickjacking, visit
the Open Web Application Security Project (OWASP) page on Clickjacking [here](https://owasp.org/www-community/attacks/Clickjacking).

## 4 Submission


You need to submit a detailed lab report, with screenshots, to describe what you have done
and what you have observed. You also need to provide explanation to the observations that are
interesting or surprising. Please also list the important code snippets followed by explanation.
Simply attaching code without any explanation will not receive credits.


