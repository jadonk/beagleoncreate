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

% Last Modified by GUIDE v2.5 04-Jan-2012 13:48:56

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
global videoON;
videoON = 0;
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
global videoON;
sendpacket(PacketType.INIT);
while(1)
    if (videoON == 1)
        [img size] = streamCreateVideo();
        if size ~= 0
            imshow(img,'DisplayRange',[0 255])
        end
    end
    tag = streamCreateARtag();
    if tag.isValid ~= 0
        set(handles.txtX,'String',num2str(tag.x));
        set(handles.txtY,'String',num2str(tag.y));
        set(handles.txtZ,'String',num2str(tag.z));
        set(handles.txtYaw,'String',num2str(tag.yaw));
    end
    dist = streamCreateSonar();
    if dist.isValid ~= 0
        set(handles.txtDist1,'String',num2str(dist.sonar1));
        set(handles.txtDist2,'String',num2str(dist.sonar2));
        set(handles.txtDist3,'String',num2str(dist.sonar3));
    end
    pause(0.1)
end


% --- Executes on button press in btnINIT.
function btnINIT_Callback(hObject, eventdata, handles)
% hObject    handle to btnINIT (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
sendpacket(PacketType.INIT);



% --- Executes on button press in btnEND.
function btnEND_Callback(hObject, eventdata, handles)
% hObject    handle to btnEND (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
sendpacket(PacketType.END);
global videoON;
videoON = 0;



% --- Executes on button press in btnSHUTDOWN.
function btnSHUTDOWN_Callback(hObject, eventdata, handles)
% hObject    handle to btnSHUTDOWN (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
sendpacket(PacketType.SHUTDOWN);
global videoON;
videoON = 0;


% --- Executes on button press in videoOn.
function videoOn_Callback(hObject, eventdata, handles)
% hObject    handle to videoOn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
global videoON;
if (videoON == 1)
    videoON = 0;
else
    videoON = 1;
end


% --- Executes on button press in btnCTRL.
function btnCTRL_Callback(hObject, eventdata, handles)
% hObject    handle to btnCTRL (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
sendpacket(PacketType.CTRL);
