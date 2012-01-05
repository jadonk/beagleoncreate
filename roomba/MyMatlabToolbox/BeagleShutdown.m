function BeagleShutdown(ports)
%BEAGLESHUTDOWN  Send a SHUTDOWN message to shutdown the sensor
%   handler on the Create robot.
%
%   The udp port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
% By: Chuck Yang, ty244, 2012
try
    fwrite(ports.beagle,PacketType.SHUTDOWN);
    pause(.5)
    fclose(ports.create);
    fclose(ports.beacon);
    fclose(ports.sonar);
    fclose(ports.beagle);
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end
end