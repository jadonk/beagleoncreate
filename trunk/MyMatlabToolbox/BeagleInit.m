function BeagleInit(ports)
%CREATEBEAGLEINIT  Send an INIT message to init the exclusive right to this
%   Create robot.
%
%   The udp port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
% By: Chuck Yang, ty244, 2012
try
    fwrite(ports.beagle,PacketType.INIT);
    pause(.5)
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end
end