function BeagleControl(ports, msg)
%BEAGLECONTROL  Send a CONTROL message to the sensor handler (beagleboard) 
%	on the Create robot.
%
%	Input argument 'msg' is the message you want to put in for the data 
%	field of the packet.
%
%   The object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
% By: Chuck Yang, ty244, 2012

packet = [PacketType.CTRL 0 0 0 '00000000' msg];
fwrite(ports.beagle, packet);

end
