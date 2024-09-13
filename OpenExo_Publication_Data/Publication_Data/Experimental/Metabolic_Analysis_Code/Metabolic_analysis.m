 %% HLO Omnia Post-Processing
%
% Include this file in a folder containing .xlsx files exported from Omnia.
% The metabolic data will be imported based on user selection. A MATLAB
% struct will be used to save metabolic data.

%% Setup

clc
clear
close all
format short g

%% Get Files and Read

[name,path]=uigetfile('*','Select metabolic File');
[a,met_name,ext]=fileparts(name);

pat = '_';
parts = regexp(met_name, pat, 'split');

%% Select Files and Withdraw Data
%Standing Baseline
run get_meta_data.m;

    for i=1:length(marker_data)
        if strcmp(marker_data{i},'baseline start')==1
            base_start=i;
        end
        if strcmp(marker_data{i},'baseline end')==1
            base_end=i;
        end
    end
    if exist('base_start')==1
    stand_base_all(:,1)=time(base_start:base_end,1);
    stand_base_all(:,2)=V02(base_start:base_end,1);
    stand_base_all(:,3)=VC02(base_start:base_end,1);
    HR_base=mean(HR(base_start:base_end));
   
    for i=2:length(stand_base_all(:,1))
        if stand_base_all(i-1,1)<=stand_base_all(end,1)-60&&stand_base_all(i,1)>stand_base_all(end,1)-60
            P=i;
        end
    end
    standbase=(16.58*stand_base_all(:,2)+4.51*stand_base_all(:,3))/60;
    standbase_all=mean(standbase(:,1));
    standbase_1min=mean(standbase(P-1:end,1));
    
    standbasenorm_all = standbase_all/BW; 
    else
        standbase_1min=117.4;
    end

%  standbase_1min=133.8;
for i=1:length(marker_data)
    if strcmp(marker_data{i},'0min')==1
        min0=i;
    elseif strcmp(marker_data{i},'1min')==1
        min1=i;
    elseif strcmp(marker_data{i},'2min')==1
        min2=i;
    elseif strcmp(marker_data{i},'3min')==1
        min3=i;
    elseif strcmp(marker_data{i},'4min')==1
        min4=i;
    elseif strcmp(marker_data{i},'5min')==1
        min5=i;
    elseif strcmp(marker_data{i},'6min')==1
        min6=i;
    elseif strcmp(marker_data{i},'stop')==1
        stop=i;
    elseif strcmp(marker_data{i},'7min')==1
        min7=i;
    elseif strcmp(marker_data{i},'8min')==1
        min8=i;
    elseif strcmp(marker_data{i},'9min')==1
        min9=i;
    elseif strcmp(marker_data{i},'10min')==1
        min10=i;
    elseif strcmp(marker_data{i},'11min')==1
        min11=i;
    elseif strcmp(marker_data{i},'12min')==1
        min12=i;
    elseif strcmp(marker_data{i},'13min')==1
        min13=i;
    elseif strcmp(marker_data{i},'14min')==1
        min14=i;
    elseif strcmp(marker_data{i},'15min')==1
        min15=i;
    elseif strcmp(marker_data{i},'16min')==1
        min16=i;
    elseif strcmp(marker_data{i},'17min')==1
        min17=i;
    elseif strcmp(marker_data{i},'18min')==1
        min18=i;
    elseif strcmp(marker_data{i},'19min')==1
        min19=i;
    elseif strcmp(marker_data{i},'20min')==1
        min20=i;
    elseif strcmp(marker_data{i},'21min')==1
        min21=i;
    elseif strcmp(marker_data{i},'22min')==1
        min22=i;
    elseif strcmp(marker_data{i},'23min')==1
        min23=i;
    elseif strcmp(marker_data{i},'24min')==1
        min24=i;
    elseif strcmp(marker_data{i},'25min')==1
        min25=i;
    elseif strcmp(marker_data{i},'26min')==1
        min26=i;
    elseif strcmp(marker_data{i},'27min')==1
        min27=i;
    elseif strcmp(marker_data{i},'28min')==1
        min28=i;
    elseif strcmp(marker_data{i},'29min')==1
        min29=i;
    elseif strcmp(marker_data{i},'exo1_start')==1
        exo1start=i;
    elseif strcmp(marker_data{i},'exo1_end')==1
        exo1end=i;
    elseif strcmp(marker_data{i},'exo2_start')==1
        exo2start=i;
    elseif strcmp(marker_data{i},'exo2_end')==1
        exo2end=i;
    elseif strcmp(marker_data{i},'shod_start')==1
        shodstart=i;
    elseif strcmp(marker_data{i},'shod_end')==1
        shodend=i;
    elseif strcmp(marker_data{i},'ZT_start')==1
        ZTstart=i;
    elseif strcmp(marker_data{i},'ZT_end')==1
        ZTend=i;
    end
end


MeT=(16.58*V02+4.51*VC02)/60;

MeT_Last_Two = (MeT(min4:exo1end) - standbase_all)/BW;

if exist('shodstart')==1
    shod_Met=(mean(MeT(shodstart:shodend))-standbase_all)/BW;
    shod_time=time(shodend)-time(shodstart);
    for i=2:length(marker_data)
     if time(i-1,1)<=time(shodend,1)-180&&time(i,1)>time(shodend,1)-180
         shodstart2=i;
     end
    end
    shod_Met2=(mean(MeT(shodstart2:shodend))-standbase_all)/BW;
    shod_time2=time(shodend)-time(shodstart2);
    HR_shod=mean(HR(shodstart:shodend));
end
if exist('exo1start')==1
    exo1_Met=(mean(MeT(exo1start:exo1end))-standbase_all)/BW;
    exo1_time=time(exo1end)-time(exo1start);
    for i=2:length(marker_data)
     if time(i-1,1)<=time(exo1end,1)-180&&time(i,1)>time(exo1end,1)-180
         exo1start2=i;
     end
    end
    exo1_Met2=(mean(MeT(exo1start2:exo1end))-standbase_all)/BW;
    exo1_time2=time(exo1end)-time(exo1start2);
    HR_exo1=mean(HR(exo1start:exo1end));
end
if exist('exo2start')==1
    exo2_Met=(mean(MeT(exo2start:exo2end))-standbase_all)/BW;
    exo2_time=time(exo2end)-time(exo2start);
    for i=2:length(marker_data)
     if time(i-1,1)<=time(exo2end,1)-300&&time(i,1)>time(exo2end,1)-300
         exo2start2=i;
     end
    end
    exo2_Met2=(mean(MeT(exo2start2:exo2end))-standbase_all)/BW;
    exo2_time2=time(exo2end)-time(exo2start2);
end
if exist('ZTstart')==1
    ZT_Met=(mean(MeT(ZTstart:ZTend))-standbase_all)/BW;
    ZT_time=time(ZTend)-time(ZTstart);
    for i=2:length(marker_data)
     if time(i-1,1)<=time(ZTend,1)-180&&time(i,1)>time(ZTend,1)-180
         ZTstart2=i;
     end
    end
    ZT_Met2=(mean(MeT(ZTstart2:ZTend))-standbase_all)/BW;
    ZT_time2=time(ZTend)-time(ZTstart2);
    HR_ZT=mean(HR(ZTstart:ZTend));
end

if exist('min1')==1
    Min1_Met = (mean(MeT(exo1start:min1))-standbase_all)/BW;
end
if exist('min2')==1
    Min2_Met = (mean(MeT(min1:min2))-standbase_all)/BW;
end
if exist('min3')==1
    Min3_Met = (mean(MeT(min2:min3))-standbase_all)/BW;
end
if exist('min4')==1
    Min4_Met = (mean(MeT(min3:min4))-standbase_all)/BW;
end
if exist('min5')==1
    Min5_Met = (mean(MeT(min4:min5))-standbase_all)/BW;
    Min6_Met = (mean(MeT(min5:exo1end))-standbase_all)/BW;
    %Last_Two_Min_Met = (mean(MeT(min4:exo1end)) - standbase_all)/BW;
end
if exist('min6')==1
    Min6_Met = (mean(MeT(min5:min6))-standbase_all)/BW;
    %Last_Two_Min_Met = (mean(MeT(min4:exo1end)) - standbase_all)/BW;
    Last_Two_Min_Met = (mean(MeT(min4:min6)) - 106.089546871792)/BW;
end
if exist('min7')==1
    Min7_Met = (mean(MeT(min6:min7))-standbase_all)/BW;
    Min8_Met = (mean(MeT(min7:exo1end))-standbase_all)/BW;
end
if exist('min8')==1
    Min8_Met = (mean(MeT(min7:min8))-standbase_all)/BW;
    Min9_Met = (mean(MeT(min8:exo1end))-standbase_all)/BW;
    %Last_Two_Min_Met = (mean(MeT(min6:exo1end)) - standbase_all)/BW;
end
if exist('min9')==1
    Min9_Met = (mean(MeT(min8:min9))-standbase_all)/BW;
    Min10_Met = (mean(MeT(min9:exo1end))-standbase_all)/BW;
end
if exist('min10')==1
    Min10_Met = (mean(MeT(min9:min10))-standbase_all)/BW;
    Min11_Met = (mean(MeT(min10:exo1end))-standbase_all)/BW;
    %Last_Two_Min_Met = (mean(MeT(min8:min10)) - standbase_all)/BW;
    %Last_Two_Met_Norm = (mean(MeT(min8:min10)) - 110.12)/BW;
end
if exist('min11')==1
    Min11_Met = (mean(MeT(min10:min11))-standbase_all)/BW;
    Min12_Met = (mean(MeT(min11:exo1end))-standbase_all)/BW;
end
if exist('min12')==1
    Min12_Met = (mean(MeT(min11:min12))-standbase_all)/BW;
    Min13_Met = (mean(MeT(min12:exo1end))-standbase_all)/BW;
end
if exist('min13')==1
    Min13_Met = (mean(MeT(min12:min13))-standbase_all)/BW;
    Min14_Met = (mean(MeT(min13:exo1end))-standbase_all)/BW;
end
if exist('min14')==1
    Min14_Met = (mean(MeT(min13:min14))-standbase_all)/BW;
    Min15_Met = (mean(MeT(min14:exo1end))-standbase_all)/BW;
end
if exist('min15')==1
    Min15_Met = (mean(MeT(min14:min15))-standbase_all)/BW;
    Min16_Met = (mean(MeT(min15:exo1end))-standbase_all)/BW;
end
if exist('min16')==1
    Min16_Met = (mean(MeT(min15:min16))-standbase_all)/BW;
    Min17_Met = (mean(MeT(min16:exo1end))-standbase_all)/BW;
end
if exist('min17')==1
    Min17_Met = (mean(MeT(min16:min17))-standbase_all)/BW;
    Min18_Met = (mean(MeT(min17:exo1end))-standbase_all)/BW;
end
if exist('min17')==1
    Min17_Met = (mean(MeT(min16:min17))-standbase_all)/BW;
    Min18_Met = (mean(MeT(min17:exo1end))-standbase_all)/BW;
end
if exist('min18')==1
    Min18_Met = (mean(MeT(min17:min18))-standbase_all)/BW;
    Min19_Met = (mean(MeT(min18:exo1end))-standbase_all)/BW;
end
if exist('min19')==1
    Min19_Met = (mean(MeT(min18:min19))-standbase_all)/BW;
    Min20_Met = (mean(MeT(min19:exo1end))-standbase_all)/BW;
end
if exist('min20')==1
    Min20_Met = (mean(MeT(min19:min20))-standbase_all)/BW;
    Min21_Met = (mean(MeT(min20:exo1end))-standbase_all)/BW;
end
if exist('min21')==1
    Min21_Met = (mean(MeT(min20:min21))-standbase_all)/BW;
    Min22_Met = (mean(MeT(min21:exo1end))-standbase_all)/BW;
end
if exist('min22')==1
    Min22_Met = (mean(MeT(min21:min22))-standbase_all)/BW;
    Min23_Met = (mean(MeT(min22:exo1end))-standbase_all)/BW;
end
if exist('min23')==1
    Min23_Met = (mean(MeT(min22:min23))-standbase_all)/BW;
    Min24_Met = (mean(MeT(min23:exo1end))-standbase_all)/BW;
end
if exist('min24')==1
    Min24_Met = (mean(MeT(min23:min24))-standbase_all)/BW;
    Min25_Met = (mean(MeT(min24:exo1end))-standbase_all)/BW;
end
if exist('min25')==1
    Min25_Met = (mean(MeT(min24:min25))-standbase_all)/BW;
    Min26_Met = (mean(MeT(min25:exo1end))-standbase_all)/BW;
end
if exist('min26')==1
    Min26_Met = (mean(MeT(min25:min26))-standbase_all)/BW;
    Min27_Met = (mean(MeT(min26:exo1end))-standbase_all)/BW;
end
if exist('min27')==1
    Min27_Met = (mean(MeT(min26:min27))-standbase_all)/BW;
    Min28_Met = (mean(MeT(min27:exo1end))-standbase_all)/BW;
end
if exist('min28')==1
    Min28_Met = (mean(MeT(min27:min28))-standbase_all)/BW;
    Min29_Met = (mean(MeT(min28:exo1end))-standbase_all)/BW;
end
if exist('min29')==1
    Min29_Met = (mean(MeT(min28:min29))-standbase_all)/BW;
    Min30_Met = (mean(MeT(min29:exo1end))-standbase_all)/BW;
end

Min_4_to_6 = (mean(MeT(min4:min6))-standbase_all)/BW;
Min_6_to_8 = (mean(MeT(min6:min8))-standbase_all)/BW;

% last1min=(mean(MeT(min5+1:min6))-standbase_1min)/BW;
% last3min=(mean(MeT(min3+1:min6))-standbase_1min)/BW;
% lastall=(mean(MeT(min1+1:min6))-standbase_1min)/BW;
% MET1=mean(MeT(min0+1:min1))-standbase_1min;
% MET2=mean(MeT(min1+1:min2))-standbase_1min;
% MET3=mean(MeT(min2+1:min3))-standbase_1min;
% MET4=mean(MeT(min3+1:min4))-standbase_1min;
% MET5=mean(MeT(min4+1:min5))-standbase_1min;
% MET6=mean(MeT(min5+1:min6))-standbase_1min;
% MET7=mean(MeT(min6+1:min7))-standbase_1min;
% 
% 
% net_met=[MET1 MET2 MET3 MET4 MET5 MET6 MET7 last1min last3min lastall]/65.7;
% net_met=net_met';
