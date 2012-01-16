function out = ReadBeacon(ports)
%READBEACON  Retrieve the most recent artag reading from a Beagleboard.
%   ReadBeacon(ports) returns
%   out.id = The id of the beacon
%   out.x = The x position of the beacon
%   out.y = The y position of the beacon
%   out.z = The z position of the beacon
%   out.yaw = The orientation of the beacon
%
%   The udp port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
%   NOTE: If a beacon requested is not seen in the view then values of NaN 
%   are substituted for real data.
%
% By: Chuck Yang, ty244, 2012

warning off all;

% Initialize preliminary return value
out.id = NaN;
out.x = NaN;
out.y = NaN;
out.z = NaN;
out.yaw = NaN;
try
    fclose(ports.beacon);
    fopen(ports.beacon);

    %read in packet and get size
    [packet size] = fread(ports.beacon);
    if size > 15
        dist.id = typecast(uint8(packet(13:16)),'int32');
        dist.x = typecast(uint8(packet(17:20)),'single');
        dist.y = typecast(uint8(packet(21:24)),'single');
        dist.z = typecast(uint8(packet(25:28)),'single');
        dist.yaw = typecast(uint8(packet(29:32)),'single');
    end
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end

end