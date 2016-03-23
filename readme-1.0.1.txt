----------------------------------------------	
             Game Socket Manager
       Copyright © 2012-2015 Fox Studio
               Version 1.0.1
            client: http://u3d.as/oJg
    server: https://github.com/FoxStudio01/GameSocket/
----------------------------------------------

Thank you for buying Game Socket Manager!

For long net connection game.
Powerful and Simple,Support all platforms!
Include demo and simple server code. ( boost c++ asio server. ) 
Complete templete client <-> server send/recv msg!


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

WebPlayer need      open cmd,input "*****/webPlayer.exe --all".
server-win32        use c++ and build by vs2012,please configure your "IP" and "port" in "server.xml".

server/  server code.

how to build it?
1. server/3rd/boost/      put boost here
2. server/3rd/openssl/    put openssl here
3. build.

Same way to build mac/linux.


last,
We hope it'll help you.
