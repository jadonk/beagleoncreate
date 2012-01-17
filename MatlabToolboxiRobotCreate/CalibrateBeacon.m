function CalibrateBeacon(ports)
%CALIBRATEBEACON  Calibrate a camera on a Beagleboard.
%   CalibrateBeacon(ports) helps you calibrate your camera beacon detection 
%   by asking you to take several readings at different, known distances, 
%   and then calculating the constant offset present in all readings.
%
%   After successful calibration, the new offset value will automatically
%   be used by ReadBeacon (both immediately in the current session, as well
%   as in all future sessions).  The offset is stored in the file 
%   'beacon_calibration.mat' in the current directory.
%
%   The port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
%   See also ReadBeacon

global BEACON_OFFSET;
BEACON_OFFSET = 0;
%TEST_DISTANCES = [0.1, 0.2, 0.3]; % meters
TEST_DISTANCES = [1/4, 1/2, 3/4, 1]*11*0.0254; % meters

% Show explanation message
disp('Starting camera beacon calibration!');
disp(' ');
disp('(NOTE: For calibration, make sure you have good lighting');

% Take a series of measurements
d_measured = zeros(size(TEST_DISTANCES));
for j = 1:length(TEST_DISTANCES)
    fprintf(2, '\n(Step %d/%d) Please place the ARtag %gcm away and then press ENTER...\n', j, length(TEST_DISTANCES), TEST_DISTANCES(j)*100);
    pause
    dist = ReadBeacon(ports);
    d_measured(j) = dist.z(1);

    fprintf(2, ' measured a sonar 1 distance of %2.2gm\n', d_measured(j));
end

% Calculate offset
BEACON_OFFSET = mean(TEST_DISTANCES - d_measured);

% Save to file so you only have to calibrate once
save beacon_calibration.mat BEACON_OFFSET;

fprintf('\n\nCalibration complete!  BEACON_OFFSET is now %gm.\n', BEACON_OFFSET);