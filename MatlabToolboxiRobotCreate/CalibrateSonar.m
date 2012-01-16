function CalibrateSonar(ports)
%CALIBRATESONAR  Calibrate 3 sonar sensors on the Beagleboard.
%   CalibrateSonar(ports) helps you calibrate your sonar sensor 
%   by asking you to take several readings at different, known distances, 
%   and then calculating the constant offset present in all readings.
%
%   After successful calibration, the new offset value will automatically
%   be used by ReadSonar (both immediately in the current session, as well
%   as in all future sessions).  The offset is stored in the file 
%   'sonar_calibration.mat' in the current directory.
%
%   The port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
%   See also ReadSonar and TestSonar.

global SONAR_OFFSET;

%TEST_DISTANCES = [0.1, 0.2, 0.3]; % meters
TEST_DISTANCES = [1/4, 1/2, 3/4, 1]*11*0.0254; % meters

% Show explanation message
disp('Starting sonar calibration!');
disp(' ');
disp('(NOTE: For calibration, it is recommended that you use a large, flat');
disp('object and hold it perpendicular to the direction the sonar sensor is facing.)');

% Take a series of measurements
d_measured = zeros(3,size(TEST_DISTANCES));
for i = 1:3
    for j = 1:length(TEST_DISTANCES)
        fprintf(2, '\n(Step %d/%d) Please place the object %gcm (%g/4 the length of letter-size paper) away from sonar %g and then press ENTER...\n', j, length(TEST_DISTANCES), TEST_DISTANCES(j)*100, j, i);
        pause
        dist = ReadSonar(ports);
        d = [dist.sonar1 dist.sonar2 dist.sonar3];
        d_measured(i,j) = d(i);

        fprintf(2, ' measured a sonar %g distance of %2.2gm\n', i, d_measured(i,j));
    end
end

% Calculate offset
SONAR_OFFSET = [mean(TEST_DISTANCES - d_measured(1,:)) mean(TEST_DISTANCES - d_measured(2,:)) mean(TEST_DISTANCES - d_measured(3,:))];

% Save to file so you only have to calibrate once
save sonar_calibration.mat SONAR_OFFSET;

fprintf('\nCalibration complete! SONAR_OFFSET is now:\n');
fprintf('%gm\n', SONAR_OFFSET);