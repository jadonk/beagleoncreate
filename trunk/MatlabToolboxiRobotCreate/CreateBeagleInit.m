function ports = CreateBeagleInit(remoteHost)
%ports = CreateBeagleInit(remoteHost)
% 'ports.create' is the udp port for commanding create
% 'ports.beagle' is the udp port for communicating with beagleboard
% 'ports.beacon' is the udp port for reading beacon (ARtag)
% 'ports.sonar' is the udp port for reading sonar
%
% This file initializes udp port for use with iRobot Create
% remoteHost is the ip address of the beagleboard
% ex. CreateBeagleInit('192.168.1.141') sets ip = '192.168.1.141'
% copy this file along with PacketType.m into MatlabToolBoxiRobotCreate
%
% An optional time delay can be added after all commands
% if your code crashes frequently.  15 ms is recommended by iRobot
%
% By: Chuck Yang, ty244, 2012

global td
td = 0.015;

createPort = 8888;
beaglePort = 8866;
artagPort = 8844;
sonarPort = 8833;

% get UDP packet from the remote host
ports.create = udp(remoteHost, createPort, 'LocalPort', createPort);
ports.beagle = udp(remoteHost, beaglePort, 'LocalPort', beaglePort);
ports.beacon = udp(remoteHost, artagPort, 'LocalPort', artagPort);
ports.sonar = udp(remoteHost, sonarPort, 'LocalPort', sonarPort);

set(ports.create,'Timeout',td)
set(ports.beacon,'Timeout',0.2)
set(ports.sonar,'Timeout',0.2)

warning off

disp('Opening connection to iRobot Create...');
fopen(ports.create);
fopen(ports.beagle);
fopen(ports.beacon);
fopen(ports.sonar);

fwrite(ports.beagle,PacketType.INIT);
pause(.5)
%% Confirm two way connumication
disp('Setting iRobot Create to Control Mode...');
% Start! and see if its alive
fwrite(ports.create,128);
pause(.1)

% Set the Create in Full Control mode
% This code puts the robot in CONTROL(132) mode, which means does NOT stop 
% when cliff sensors or wheel drops are true; can also run while plugged 
% into charger
fwrite(ports.create,132);
pause(.1)

% light LEDS
fwrite(ports.create,[139 25 0 128]);

% set song
fwrite(ports.create, [140 1 1 48 20]);
pause(0.05)

% sing it
fwrite(ports.create, [141 1])

disp('I am alive if my two outboard lights came on')

% confirmation = (fread(ports.create,4))
pause(.1)
end