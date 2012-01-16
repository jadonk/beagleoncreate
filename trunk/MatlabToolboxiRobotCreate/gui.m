function varargout = gui(varargin)
% GUI M-file for gui.fig
%      GUI, by itself, creates a new GUI or raises the existing
%      singleton*.
%
%      H = GUI returns the handle to a new GUI or the handle to
%      the existing singleton*.
%
%      GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in GUI.M with the given input arguments.
%
%      GUI('Property','Value',...) creates a new GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before gui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help gui

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
    fprintf(2, '??? Too few input argument, use gui(ports) to run this.\n');
else
    ports = varargin{1};
end
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
close(gcf);


% --- Executes on button press in btnCalibCamera.
function btnCalibCamera_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibCamera (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
sendpacket(PacketType.INIT);



% --- Executes on button press in btnCalibSonar.
function btnCalibSonar_Callback(hObject, eventdata, handles)
% hObject    handle to btnCalibSonar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in btnDone.
function btnDone_Callback(hObject, eventdata, handles)
% hObject    handle to btnDone (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global DONE;
DONE = 1;
