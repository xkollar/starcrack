StarCrack!
==========

This program is licensed under GPL 2.

This program is simple bruteforcing tool. You are more likely to
achieve better performance if you use specialized program.
For example [RarCrack! fork by hyc](https://github.com/hyc/rarcrack) use libunrar thus
saving unnecessary execs. Or password cracking classic JohnTheRipper.

Currently supporting only rar archives, however, I plan to add way to bruteforce
anything that can have password specified at command line and announce its success
via return code (with presets for rar, zip, and 7z).

Pledge: This program is a toy... do not use toys for illegal purposes.

Todo
----

* Remove libxml2 dependency.
* Support wordlists.
* Generalize tool to support anything that can have password
  supplied on command line and exits with sane exit code.

Changelog
---------

Version 0.2 [this is the latest version]

* Added multiple cracking threads support
* Added 7z, zip support
* Archive file type auto-detection

Version 0.1

* Initial version (rar cracking working)

Software requirements
---------------------

* \> glibc 2.4
* any POSIX compatible operating system [sorry Window$ isn't]
* pthreads
* libxml2
* and finally: 7zip, unrar, unzip

Building and installing
-----------------------

Everything is very easy:

~~~~ sh
tar -xjf starcrack-VERSION.tar.bz2
cd starcrack-VERSION
#you need gcc or any C compiler (edit Makefile CC=YOUR_C_COMPILER)
make
#you must be root in next step:
make install
~~~~


Using StarCrack!
----------------

~~~~ sh
starcrack your_encrypted_archive.ext [--threads thread_num] [--type rar|zip|7z]
~~~~


Everything in [] is optional. By default StarCrack! use two threads and
autodetect the archive type. If the detection does not work you can
specify the correct file type with the type parameter. StarCrack! currently
crack maximum in 12 threads.

After the cracking started StarCrack! will print the current status
of cracking and save it's to a status file. If you want more specific
password character set, you need to run StarCrack! to create the XML status
file (3 sec).

~~~~ xml
<?xml version="1.0" encoding="UTF-8"?>
<starcrack>
 <abc>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ</abc>
 <current>uU</current>
 <good_password></good_password>
</starcrack>
~~~~

This is a sample XML file, and you see there is a character set. If you
want, you can modify this file and when you start StarCrack! again the
program will be use new variables. Warning: Take care when you changing
this file, make sure the current password don't have characters outside
the abc[character set]!
