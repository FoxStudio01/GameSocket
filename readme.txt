﻿----------------------------------------------	
             Game Socket Manager
       Copyright © 2012-2017 Fox Studio
               Version 1.1.1
            client: http://u3d.as/oJg
    server: https://github.com/FoxStudio01/GameSocket/
	    email:55909028@qq.com
----------------------------------------------

Thank you for buying Game Socket Manager!

For long net connection game.( TCP/IP )
Powerful and Simple,Support all platforms!
Include demo and simple server code. ( boost c++ asio server. ) 
Complete templete client <-> server send/recv msg!




--------------------
v 1.1.1
support  dns and ipv6.

platforms      safe     unsafe         dns
webgl           o         o             x
webplayer       o         x             o
pc/mac          o         o             o
ios             o         o             o
android         o         o             o


--------------------
client:

Support all platforms, webGL use webSocket.
You can use zip to improve the network transmission speed.( "GameDefine.USE_ZIP" )
Monitor the network connection and disconnection events.Just like "onConnect" and "onDisconnect" in "GameManager.cs" .

WebPlayer use c# safe code,you must write "initNetHead" and "initWithBytes" in net msg struct.
Other platform can use c# unsafe code( macro "USE_UNSAFE" ),don't need to like WebPlayer.It's more convenient for developers.
It's easy,isn't it?

Now copy folder "FoxSDK" and "Plugins" to you own projects.Start it！


--------------------
server:

WebPlayer need this cmd : "sockpol.exe --all". you can find "sockpol.exe" in unity3d "Unity\Editor\Data\Tools\SocketPolicyServer"
server-win32        use c++ and build by vs2012,please configure your "IP" and "port" in "server.xml".

server/  server code.

how to build it?
0. build boost
1. server/3rd/boost/      put boost here
2. server/3rd/openssl/    put openssl(32bit) here
3. build.

Same way to build mac/linux.


last,
We hope it'll help you.
