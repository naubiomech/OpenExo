clear all
clc

%% Read File

cd('DATA LOCATION') %This needs to be updated with the location of the file 

file_name = 'Exo_Metabolic.csv';

data = readmatrix(file_name);

%% Import Data

V02 = data(27:end,5);            %V02 Data
VC02 = data(27:end,6);           %VC02 Data
Markers = data(27:end,38);       %Marker Data: 0-100 = Baseline; 200-300 = Trail; Singal #s =  Mintues
BW = data(23,2);

clear data

%% Calculate MET

MeT = (16.58*V02+4.51*VC02)/60;

%% Sort Data and Calculate Metabolic Power

n = 1;

%Find Start and Stop Idx for Baseline and Trial
for i = 1:length(Markers)

    if Markers(i) == 100
        Baseline_Start = i;     %Idx for the End of Baseline 
    end

    if Markers(i) == 200
        Trial_Start = i;        %Idx for the Start of the Trial
    end

    if Markers(i) == 300
        Trial_End = i;          %Idx for the End of the Trial
    end

    if Markers(i) == n
        Minutes(n) = i;         %Idx for Each Minute Since Start
        n = n+1;
    end

end

%Baseline
standing_baseline = MeT(1:Baseline_Start);
standing_baseline_all = mean(standing_baseline);
standing_baseline_all_normalized = standing_baseline_all/BW;

%MeT - Averages
MeT_All = mean((MeT(Trial_Start:Trial_End) - standing_baseline_all)/BW);                %Average MeT across entire trial
MeT_Minus_First_Four = mean((MeT(Minutes(4):Trial_End) - standing_baseline_all)/BW);    %Average MeT across all but first four minutes of trial
MeT_Last_Five = mean((MeT((Minutes(end)-5):Trial_End) - standing_baseline_all)/BW);     %Average MeT across all but first four minutes of trial

%MeT - Minute by Minute 
for i = 1:length(Minutes)

    if i == 1
        MeT_Minutes(i) = mean((MeT(Trial_Start:Minutes(i)) - standing_baseline_all)/BW);
    end

    if i ~= 1
        MeT_Minutes(i) = mean((MeT(Minutes(i-1):Minutes(i)) - standing_baseline_all)/BW);
    end

end

MeT_Minutes(length(Minutes)+1) = mean((MeT(Minutes(end):Trial_End) - standing_baseline_all)/BW);

MeT_Minutes = MeT_Minutes';

%CoT - Minute by Minute 
for i = 1:length(Minutes)

    if i == 1
        CoT_Minutes(i) = mean((MeT(Trial_Start:Minutes(i)) - standing_baseline_all)/BW)/1.33;
    end

    if i ~= 1
        CoT_Minutes(i) = mean((MeT(Minutes(i-1):Minutes(i)) - standing_baseline_all)/BW)/1.33;
    end

end

CoT_Minutes(length(Minutes)+1) = mean((MeT(Minutes(end):Trial_End) - standing_baseline_all)/BW)/1.33;

CoT_Minutes = CoT_Minutes';

%MeT Normalized Throughout 
Normalized_MeT = (MeT(Trial_Start:Trial_End) - standing_baseline_all)/BW;

save('COT_MeT.mat', 'Normalized_MeT');