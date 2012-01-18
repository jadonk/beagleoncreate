function varargout = CalibGUI(varargin)
% CALIBGUI M-file for CalibGUI.fig
%      CalibGUI, by itself, creates a new GUI or raises the existing
%      singleton*.
%
%      H = CalibGUI(ports) returns the handle to a new GUI or the handle to
%      the existing singleton*.
%
%      CalibGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in CalibGUI.M with the given input arguments.
%
%      CalibGUI('Property','Value',...) creates a new GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before gui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to gui_OpeningFcn via varargin.
%
%      *See CalibGUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: CreateBeagleInit, GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help CalibGUI

% Last Modified by GUIDE v2.5 16-Jan-2012 15:44:27

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @gui_OpeningFcn, ...
                   'gui_OutputFcn',  @gui_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before gui is made visible.
function gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to gui (see VARARGIN)

% Choose default command line output for gui
handles.output = hObject;
global DONE;
DONE = 0;
global ports;
if isempty(varargin)
    DONE = 1;
    fprintf(2, '??? Too few input argument, use CalibGUI(ports) to run this.\n');
else
    ports = varargin{1};
end

% send CTRL msg to beagleboard to enable video streaming
BeagleControl(ports, 3);

global TEST_DISTANCES;
TEST_DISTANCES = [1/4, 1/2, 3/4, 1]*11*0.0254; % meters
global d_measured;
d_measured = zeros(3,length(TEST_DISTANCES));
global sonarCalib;
sonarCalib = 0;
global SONAR_OFFSET;
SONAR_OFFSET = [0 0 0];

global cam_measured;
cam_measured = zeros(size(TEST_DISTANCES));
global cameraCalib;
cameraCalib = 0;
global BEACON_OFFSET;
BEACON_OFFSET = 0;
% Update handles structure
guidata(hObject, handles);
% UIWAIT makes gui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = gui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
global ports;
global DONE;
global dist;
global tag;

while(DONE == 0)
    
    PlotCreateVideo(1);

    tag = ReadBeacon(ports);
    tagInd = find(~isnan(tag.id));
    if ~isempty(tagInd)
        set(handles.txtX,'String',num2str(tag.x(tagInd(1))));
        set(handles.txtY,'String',num2str(tag.y(tagInd(1))));
        set(handles.txtZ,'String',num2str(tag.z(tagInd(1))));
        set(handles.txtYaw,'String',num2str(tag.yaw(tagInd(1))));
    end
    dist = ReadSonar(ports);
    set(handles.txtDist1,'String',num2str(dist.sonar1));
    set(handles.txtDist2,'String',num2str(dist.sonar2));
    set(handles.txtDist3,'String',num2str(dist.sonar3));

    pause(0.1)
end
% send CTRL msg to beagleboard to disable video streaming
BeagleControl(ports, 4);
close(gcf);


% --- Executes on button press in btnCalibCamera.
function btnCalibCamera_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibCamera (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global BEACON_OFFSET;
global tag;
global cameraCalib;
global TEST_DISTANCES;
global cam_measured;

j = mod(cameraCalib,length(TEST_DISTANCES)) + 1;
cameraCalib = cameraCalib + 1;
startstr = sprintf('');
if cameraCalib == 1
    BEACON_OFFSET = 0;
    % Show explanation message
    startstr = sprintf('\nStarting camera beacon calibration!\n(NOTE: For calibration, make sure you have good lighting');
end
str = strcat(startstr,sprintf('\n(Step %d/%d) Please place the ARtag %gcm away', j, length(TEST_DISTANCES), TEST_DISTANCES(j)*100));
set(handles.txtMsg,'String', str);

if cameraCalib == length(TEST_DISTANCES)
    set(handles.btnCalibCamera,'String', 'Done Calibration');
else
    set(handles.btnCalibCamera,'String', 'Next');
end

% Camera Calibration
if cameraCalib > 1
    j = mod(cameraCalib-2,length(TEST_DISTANCES)) + 1;
    cam_measured(j) = tag.z(1);

    if cameraCalib == length(TEST_DISTANCES)+1
        cameraCalib = 0;
        % Calculate offset
        BEACON_OFFSET = mean(TEST_DISTANCES - cam_measured);
        cam_measured = zeros(size(TEST_DISTANCES));
        % Save to file so you only have to calibrate once
        save beacon_calibration.mat BEACON_OFFSET;
        str = sprintf('\nCalibration complete!  BEACON_OFFSET is now %gm.\n', BEACON_OFFSET);
        set(handles.txtMsg,'String', str);
        set(handles.btnCalibCamera,'String', 'Calibrate Camera');
    end
end



% --- Executes on button press in btnCalibSonar.
function btnCalibSonar_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibSonar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global SONAR_OFFSET;
global dist;
global sonarCalib;
global TEST_DISTANCES;
global d_measured;

j = mod(sonarCalib,length(TEST_DISTANCES)) + 1;
sonarCalib = sonarCalib + 1;
i = ceil(sonarCalib/length(TEST_DISTANCES));
startstr = sprintf('');
if sonarCalib == 1
    SONAR_OFFSET = [0 0 0];
    % Show explanation message
    startstr = sprintf('\nStarting sonar calibration!\n(NOTE: For calibration, it is recommended that you use a large, flat\nobject and hold it perpendicular to the direction the sonar sensor is facing.)');
end
str = strcat(startstr,sprintf('\nPlease place the object %gcm (%g/4 the length of letter-size paper) away from sonar %g', TEST_DISTANCES(j)*100, j, i));
set(handles.txtMsg,'String', str);

if sonarCalib == 3*length(TEST_DISTANCES)
    set(handles.btnCalibSonar,'String', 'Done Calibration');
else
    set(handles.btnCalibSonar,'String', 'Next');
end

% Sonar Calibration
if sonarCalib > 1
    j = mod(sonarCalib-2,length(TEST_DISTANCES)) + 1;
    i = ceil((sonarCalib-1)/length(TEST_DISTANCES));
    d = [dist.sonar1 dist.sonar2 dist.sonar3];
    d_measured(i,j) = d(i);

    if sonarCalib == 3*length(TEST_DISTANCES)+1
        sonarCalib = 0;
        % Calculate offset
        SONAR_OFFSET = [mean(TEST_DISTANCES - d_measured(1,:)) mean(TEST_DISTANCES - d_measured(2,:)) mean(TEST_DISTANCES - d_measured(3,:))];
        d_measured = zeros(3,length(TEST_DISTANCES));
        % Save to file so you only have to calibrate once
        save sonar_calibration.mat SONAR_OFFSET;
        str = sprintf('\nCalibration complete! SONAR_OFFSET is now [%gm %gm %gm]', SONAR_OFFSET(1), SONAR_OFFSET(2), SONAR_OFFSET(3));
        set(handles.txtMsg,'String', str);
        set(handles.btnCalibSonar,'String', 'Calibrate Sonar');
    end
end



% --- Executes on button press in btnDone.
function btnDone_Callback(hObject, eventdata, handles)
% hObject    handle to btnDone (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global DONE;
DONE = 1;
