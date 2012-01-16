function dist = ReadSonar(ports)
%READSONAR  Retrieve the most recent sonar reading from a Beagleboard.
%   ReadSonar(ports) returns the last sonar distance reading that was 
%   captured by a Beagleboard attached to an iRobot Create or Roomba, in 
%   meters.
%   dist.sonar1 = distance measurement of sonar1
%   dist.sonar2 = distance measurement of sonar2
%   dist.sonar3 = distance measurement of sonar3
%
%   The udp port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%   
%   NOTE: The first time a new sonar sensor is used, it is recommended 
%   that you calibrate it using the CalibrateSonar function.
%
%   See also CalibrateSonar and TestSonar.
%
% By: Chuck Yang, ty244, 2012

warning off all;

global SONAR_OFFSET;
if isempty(SONAR_OFFSET)
SONAR_OFFSET = [-0.12 -0.12 -0.12];
end

% Initialize preliminary return value
dist.sonar1 = NaN;
dist.sonar2 = NaN;
dist.sonar3 = NaN;
try
    fclose(ports.sonar);
    pause(.01);
    fopen(ports.sonar);

	[packet size] = fread(ports.sonar);
	if size > 14
		dist.sonar1 = typecast(uint8(packet(13:16)),'single');
		dist.sonar2 = typecast(uint8(packet(17:20)),'single');
		dist.sonar3 = typecast(uint8(packet(21:24)),'single');
        if dist.sonar1 > 0
            dist.sonar1 = dist.sonar1 + SONAR_OFFSET(1);
        else
            dist.sonar1 = NaN;
        end
        if dist.sonar2 > 0
            dist.sonar2 = dist.sonar2 + SONAR_OFFSET(2);
        else
            dist.sonar2 = NaN;
        end
        if dist.sonar3 > 0
            dist.sonar3 = dist.sonar3 + SONAR_OFFSET(3);
        else
            dist.sonar3 = NaN;
        end
    end
catch err
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
    err.identifier
end

end