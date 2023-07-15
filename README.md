StarCrack!
==========

This program is licensed under GPL 2.

This program is simple brute-force tool. You are more likely to
achieve better performance if you use specialized program.
For example [RarCrack! fork by hyc](https://github.com/hyc/rarcrack) use libunrar thus
saving unnecessary forks/execs. Or password cracking classic JohnTheRipper.

Currently supporting only rar archives, however, I plan to add way to bruteforce
anything that can have password specified at command line and announce its success
via return code (with presets for rar, zip, and 7z).

Pledge: This program is a toy, do not use toys for illegal purposes.

TODO
----

* Remove `libxml2` dependency.
* Support wordlists.
* Generalize tool to support anything that can have password
  supplied on command line and exits with sane exit code.
* Probably rewire from the scratch with code reuse.

Changelog
---------

Version 0.3  [this is the latest version]

* Support for 7z and zip was disabled for faster
  rar support. Yes, this is silly and should
  be easy to fix, not hyped about working on
  it, PRs are welcome.

Version 0.2

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


Everything in [] is optional. By default StarCrack! uses two threads and
autodetects the archive type. If the detection does not work you can
specify the correct file type with the type parameter. StarCrack! currently
cracks in at most 12 threads.

StarCrack! will print it's current progress/status and also maintains it's
state in a state file. To specify custom character set for the password, edit
it's state file (will be auto-generated on first run after about 3 seconds).

Alternatively use the following example XML file and update as you need it.
Warning: Take care when you changing
this file, make sure the `<current>` password consists only of characters
specified in `<abc>`!

~~~~ xml
<?xml version="1.0" encoding="UTF-8"?>
<starcrack>
 <abc>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ</abc>
 <current>uU</current>
 <good_password></good_password>
</starcrack>
~~~~
