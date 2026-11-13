%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file example_m6.m
%  ``````````````````
%  Accessing Mosel arrays
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014, rev. Sep. 2018
%*******************************************************

mos={
'model example                                                  '
' public declarations                                           '
'  CITIES = {"london", "paris", "madrid", "rome", "florence"}   '
'  ZONES = {"north", "south", "east", "west"}                   '
'  VALUE: dynamic array(CITIES,ZONES) of real                   '
' end-declarations                                              '
'                                                               '
' VALUE("london", "east")  := 1                                 '
' VALUE("rome",   "west")  := 2                                 '
' VALUE("paris",  "south") := 3                                 '
' VALUE("madrid", "east")  := 4                                 '
'end-model                                                      '
};

mosel = com.dashoptimization.XPRM; 
mosel.compile('', 'matlab.mws:mos', 'example_m6.bim');
mod = mosel.loadModel('example_m6.bim');
mod.run;

value = mod.findIdentifier('VALUE'); 
value_iter = value.indices(true);
sets = value.getIndexSets();
while value_iter.hasNext    
  indices = value_iter.next;
  fprintf(1, 'VALUE ( ');
  for i=1:size(indices,1)
    fprintf(1, '%s ', char(sets(i).get(indices(i))));
  end
  fprintf(1, ') = %g\n', value.getAsReal(indices));
end
