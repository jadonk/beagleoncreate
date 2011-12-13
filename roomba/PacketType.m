classdef (Sealed) PacketType

properties  (Constant)
    INIT = 1;
    END = 2;
    CTRL = 3;
    DATA = 4;
    ERROR = 5;
    SHUTDOWN = 6;
    UNKNOWN = 7;
end %constant properties
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
methods (Access = private)
%private so that you can't instatiate.
    function out = PacketType
    end %PacketType()
end %private methods
end %class PacketType