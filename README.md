jrerun
======
An executable launcher for Java applications.

The current project page is located here: <https://github.com/bbrice/jrerun>

Motivation
----------
I made this utility to launch Java applications without the need to open a
command prompt or have some kind of file association to `javaw.exe`.

Building
--------
At the moment, Visual Studio 2017 is used for this project.

Usage
-----
The `jrerun` application looks for an `ini` file that shares the same name as
the `exe` file.  This lets you name the `exe` to anything, but just make sure
the `ini` file matches.

If the `jrerun` application file is named `MyApp.exe`, then `MyApp.ini` needs
to exist for configuration purposes.

Here is a sample `ini` file setup to launch Minecraft:

	[app]
	args=-Xmx128m -jar
	file=C:\Users\Public\Games\Minecraft\Minecraft.exe

*	`app.args`
	`javaw.exe` arguments
*	`app.file`
	Path to the Java application

Any command-line arguments passed to `jrerun` will be passed as arguments to
the Java applications.  Buildling on the above Minecraft sample, here'd be how
to pass arguments to `jrerun`:

	jrerun.exe username password

License
-------
`jrerun` is licensed under the BSD license. See [LICENSE.txt](LICENSE.txt)
for more info.
