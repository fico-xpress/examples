%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file ugsol.m
%  ``````````````````
%  Accessing modeling objects and solution information.
%  Running the BIM file.
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014
%*******************************************************

mos={
'model Burglar_m                                                         '
' uses "mmxprs"                                                          '
' public declarations                                                    '
'  WTMAX = 102                    ! Maximum weight allowed               '
'  ITEMS: range                                                          '
'  VALUE: array(ITEMS) of real    ! Value of items                       '
'  WEIGHT: array(ITEMS) of real   ! Weight of items                      '
'  take: array(ITEMS) of mpvar    ! 1 if we take item i; 0 otherwise     '
' end-declarations                                                       '
'                                                                        '
' initializations from "matlab.mws:"                                     '
'  VALUE                                                                 '
'  WEIGHT                                                                '
' end-initializations                                                    '
'                                                                        '
' MaxVal:= sum(i in ITEMS) VALUE(i)*take(i)  ! Objective: max total value'
' sum(i in ITEMS) WEIGHT(i)*take(i) <= WTMAX ! Weight restriction        '
' forall(i in ITEMS) take(i) is_binary       ! All variables are 0/1     '
' maximize(MaxVal)                           ! Solve the MIP-problem     '
'                                                                        '
' initializations to "matlab.mws:"                                       '
'  evaluation of array(i in ITEMS) take(i).sol as "TAKE"                 '
' end-initializations                                                    '
'end-model                                                               '
};

ITEMS ={'camera' 'necklace' 'vase' 'picture' 'tv' 'video' 'chest' 'brick'};
VALUE =[ 15       100        90     60        40   15      10      1];
WEIGHT=[ 2        20         20     30        40   30      60      10];

mosel=com.dashoptimization.XPRM; % Initialize Mosel
mosel.compile('', 'matlab.mws:mos', 'burglar_m.bim');
mod=mosel.loadModel('burglar_m.bim');
mod.run;
if mod.getProblemStatus~=mod.PB_OPTIMAL, return, end
fprintf(1,'Objective value: %g\n', mod.getObjectiveValue); % show objective
table(ITEMS',logical(TAKE),VALUE','VariableNames',{'Item' 'Take' 'Value'})
fprintf(1,'Calculated objective: %g\n', VALUE*TAKE); % verify sol
mod.finalize
