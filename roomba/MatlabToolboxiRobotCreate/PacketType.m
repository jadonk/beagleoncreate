classdef (Sealed) PacketType

properties  (Constant)
    INIT = 1;
    END = 2;
    CTRL = 3;
    DATA = 4;
    ERROR = 5;
	CREATE = 6;
    SHUTDOWN = 7;
    UNKNOWN = 8;
end %constant properties
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
methods (Access = private)
%private so that you can't instatiate.
    function out = PacketType
    end %PacketType()
end %private methods
end %class PacketType