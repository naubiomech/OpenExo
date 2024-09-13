% Filt_Spec='*.xlsx';
% [file_name,path_name]=uigetfile(Filt_Spec,'Select Metabolic File');

try
meta=importdata(fullfile(path,name))
V02=meta.data.Data(4:end,14)
VC02=meta.data.Data(4:end,15)
marker_data=meta.textdata.Data(4:end,35)
time=meta.data.Data(4:end,9)*24*60*60;
BW=meta.data.Data(7,1);
HR=meta.data.Data(4:end,23)
catch
meta = readtable(fullfile(path,name));
V02=str2double(meta.VO2(3:end));
VC02=str2double(meta.VCO2(3:end));
marker_data=meta.Marker(3:end);
BW=str2double(meta.Var2(6));
end

MeT = (16.58*V02+4.51*VC02)/60;

plot(time,V02)
for i=1:length(marker_data)
txt1 = strcat('\leftarrow',marker_data(i));
if ~strcmp(txt1,'\leftarrow')
h=text(time(i),V02(i),txt1,'FontSize',10);
set(h,'Rotation',90);
end
end
xlabel('Time (s)')