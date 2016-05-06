# Registry Grep #

There is already a Registry Grep tool [here](http://www.softpedia.com/get/Tweak/Registry-Tweak/Registry-Grep.shtml). But I don't like it. I wrote my own regrep.py using python and pywin32. It's CLI version, I have no interest to make it GUI.

Usage is very simple:
```
$regrep.py "hello.*world"
```
will search that pattern in your registry. Read the code at [here](http://windows-config.googlecode.com/svn/trunk/bin/windows/regrep.py)

# Registry Subst #

It's just one step further from regrep.py to regsub.py. In fact, I wrote regsub.py first, because I have a task to perform: I need to change the user profile folder from `c:\documents and settings` to `d:\profiles`, this way, after I re-install windows, I still have my user data and configuration files. I don't need to remember to backup before I reinstall.

I did it with:
```
$regsub.py 'c:.*documents and settings' 'd:/profiles'
```

Read the code at [here](http://windows-config.googlecode.com/svn/trunk/bin/windows/regsub.py)

# Registry Open #

```
$regopen.py 'HKEY_CURRENT_USER\Software\JavaSoft\Prefs\org\antlr\works'
```
will open that key in regedit.exe.

The code is at [here](http://windows-config.googlecode.com/svn/trunk/bin/windows/regopen.py).