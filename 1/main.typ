#set document(
  title: [Task №1: #link("https://app.hackthebox.com/machines/Fireflow")[Fireflow machine] \ Matvey Zenin 24214],
)
#set page(
  paper: "a4",
  numbering: "1",
)
#set text(
  font: "New Computer Modern",
  size: 12pt,
)

#set heading(numbering: "1.")
#show link: underline

#align(center, title())

= Reconnaissance

Check if target is reachable:

#figure(
  image("pictures/1.png"),
  caption: [Pinging target]
)

== Nmap

First of all, let's check which ports are open on the target and which soft is listening on each of them. To do this we will use Nmap with options:
- `-sV` - check version of the soft listening on each opened port
- `-sC` - run default nmap script on found services

#figure(
  image("pictures/2.png"),
  caption: [Using Nmap on target]
)

Now we know, that there are:
- ssh on port 22
- nginx on port 443

So, the next step is trying to inspect the website behind the nginx

== Website inspection

Nginx uses self-signed certificate (also, certificate belongs to domain name, but we are connecting with ip as hostname), so we need to trust it in order to connect:

#figure(
  image("pictures/3.png"),
  caption: [Trusting targets' self-signed certificate]
)

After that, since we are using ip as a hostname, nginx redirects us to `fireflow.htb`, which cannot be resolved, so we fail to connect to it:

#figure(
  image("pictures/4.png"),
  caption: [Connection failure after redirect]
)

To solve this problem, let's add ip for this domain to the `/etc/hosts` file:

#figure(
  image("pictures/5.png"),
  caption: [Adding `fireflow.htb` ip to the `/etc/hosts`]
)

After that (and trusting the same certificate, but now for the actual server domain name), we successfully got a website main page:

#figure(
  image("pictures/6.png"),
  caption: [`fireflow.htb`]
)

After pressing orange button we got redirected to the subdomain `flow.fireflow.htb`, after adding ip for that subdomain to the `/etc/hosts` as we've done before, we see following webpage:

#figure(
  image("pictures/7.png"),
  caption: [`flow.fireflow.htb/playground/7d84d636-af65-42e4-ac38-26e867052c25`]
)

As we can see, it's chat with LLM built with Langflow, let's check the version of Langflow used with curl (option `-k` is used to skip certificate verification):

#figure(
  image("pictures/8.png"),
  caption: [Getting Langflow version used on target]
)

Langflow with version $<=$ 1.8.2 is known to be vulnerable to remote code execution #link("https://github.com/advisories/GHSA-vwmf-pq79-vjvx")[CVE-2026-33017], so if we obtain any public flow id, we will be able to execute arbitary code on the target and get a reverse shell.

The public flow id used for testing purposes can be obtained from the URL of the page we are at: `flow.fireflow.htb/playground/7d84d636-af65-42e4-ac38-26e867052c25` (the last part with UUID is the flow id)

= Weaponization

Now, we need a script to deliver malicious code to the target using this flow id. We will use #link("https://github.com/oscar-mine/CVE-2026-33017-Exploit")[this script].

= Delivery + exploitation

Start netcat in listening mode on port 4444, to which target will connect its' shell
#figure(
  image("pictures/16.png"),
  caption: [Starting netcat]
)

First script usage with flow id found earlier failed:
#figure(
  image("pictures/15.png"),
  caption: [Script failure]
)

As we can see, the script tries to connect to the target with `requests` library, which refuse to connect because nginx on target provides self-signed certificate (yet again):
#figure(
  image("pictures/11.png"),
  caption: [Script sends request with certidicate verification turned on]
)

Let's fix that by adding `verify=False` argument:
#figure(
  image("pictures/12.png"),
  caption: [Script sends request with certidicate verification turned on]
)

Now the script is working:
#figure(
  image("pictures/13.png"),
  caption: [Successful script usage]
)

We successfully obtained shell as `www-data` user:
#figure(
  image("pictures/14.png"),
  caption: [`www-data` user shell]
)

