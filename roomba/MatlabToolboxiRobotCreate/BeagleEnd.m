function BeagleEnd(ports)
%CREATEBEAGLEEND  Send an END message to end the exclusive right to this
%   Create robot.
%
%   The udp port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
% By: Chuck Yang, ty244, 2012
try
    fwrite(ports.beagle,PacketType.END);
    pause(.5)
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end
end