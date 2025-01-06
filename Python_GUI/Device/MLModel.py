import numpy as np
import pandas as pd
from glob import glob
from os.path import basename
from os import remove
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import cross_val_score, KFold, train_test_split, GridSearchCV
from sklearn.inspection import permutation_importance
from sklearn.metrics import ConfusionMatrixDisplay, accuracy_score
from micromlgen import port
import matplotlib.pyplot as plt
from collections import OrderedDict
import pickle
from tkinter import StringVar


class MLModel:
    def __init__(self):
        self.database=[] #define our data storage variable
        self.state=0 #recording flag, 0 = not recording, 1=level, 2=descend, 3=ascend
        self.randomstate=30 #used to control random shuffling of machine learning parameters
        self.nestedscore=-1 #store the generalized model accuracy
        self.modelName="model.pkl" #name our model file
        
        self.prediction=0
        self.modelStatus=StringVar()
        self.levelSteps=0
        self.descendSteps=0
        self.ascendSteps=0
        self.levelStepsLabel=StringVar()
        self.levelStepsLabel.set(str(self.levelSteps))
        self.descendStepsLabel=StringVar()
        self.descendStepsLabel.set(str(self.descendSteps))
        self.ascendStepsLabel=StringVar()
        self.ascendStepsLabel.set(str(self.ascendSteps))
        self.controlModeLabel=StringVar()
        self.controlMode=0
        

        try: #check whether or not we already have a model made previously
            with open(self.modelName, 'rb') as file:
                self.model=pickle.load(file) #load the model
            self.optimizedscore=round(self.model.best_score_*100,1) #record its score
            self.modelExists=1 #flag that our model exists
            self.modelStatus.set("Model Loaded " + str(self.optimizedscore) +"% Acc") #tell user model is already loaded
        except: #if we don't already have a model load everything as default blank
            self.model=[] 
            self.optimizedscore=-1
            self.modelExists=0
            self.modelStatus.set("Model Inactive ")
            pass
        
            

    def addDataPoints( #add a sample to our database
        self,
        newsample

    ):

        if not self.state==0: #if we are in a data recording mode
            if not len(self.database) or not (self.database[-1]==newsample):#if model database is empty, or if the current sample has not already been added to the database
                self.database.extend([newsample]) #add that sample
                if self.state == 1:
                    self.levelSteps +=1
                    self.levelStepsLabel.set(str(self.levelSteps))
                elif self.state == 2:
                    self.descendSteps +=1
                    self.descendStepsLabel.set(str(self.descendSteps))
                else:
                    self.ascendSteps +=1
                    self.ascendStepsLabel.set(str(self.ascendSteps))
            
    def createModel( #create the Machine learning model
        self
    ):
        data=np.array(self.database) #grab that data
        features, labels = data[:, :-1], data[:, -1] #split into our features and labels
        param_grid = [{'max_depth': [1, 5, 10],
                'n_estimators': [ 30, 40, 50],
                'min_samples_leaf': [3,  11],
                'max_leaf_nodes': [3, 11],
                }] #create hyperparameter grid space, options for model to test which config = best accuracy
        
        if len(data[:,1]) <12: #if we have fewer than 12 samples
            numsplits=2 #set the number of validation splits = minimum possible
        else:
            numsplits=10 #otherwise max out at 10 validation splits,
        #we want our inner and outer folds to have at leats 10 splits, using 12 as the cut offs ensures that after the first hold one out fold, 
        #we still have 10 samples for the inner fold. If we chose 11, one layer would have 2 samples and the other 9 would have 1 sample, 
        #so the inner fold would fail saying 9 samples present, but need 10 for an even split 
        innerkf=KFold(n_splits=numsplits, shuffle=True, random_state=self.randomstate) #create our folds for double cross validation
        outerkf=KFold(n_splits=numsplits, shuffle=True, random_state=self.randomstate)
        
        #create GridSearch optimizer object
        """     
        nestedoptimization=GridSearchCV(RandomForestClassifier(random_state=self.randomstate),param_grid, return_train_score=True,cv=innerkf, scoring='accuracy')
        cross_val_results = cross_val_score(nestedoptimization, features, labels, cv=outerkf) #peform the nested cross fold validation, test accuracy of model
        self.nestedscore= round(cross_val_results.mean()*100,1) #check the average accuracy of every fold """
        #recreate optimizer for separate fitting
        optimization=GridSearchCV(RandomForestClassifier(random_state=self.randomstate),param_grid, return_train_score=True,cv=outerkf, scoring='accuracy') #create optimization object 
        optimization.fit(features,labels) #test all model options
        self.optimizedscore=round(optimization.best_score_*100,1) #record the best score
        self.model=optimization #store the best model
        self.modelExists=1 #flag that our model exists 
        self.trainscore=optimization.score(features,labels)
        self.modelStatus.set("Model Loaded "+ str(self.optimizedscore) +"% Acc") #tell user model is loaded
        
        with open(self.modelName, 'wb') as file:
            pickle.dump(optimization,file) #save our model
        
    def predictModel(self,newsample): 
        if self.modelExists: #and if the model exists
            self.prediction=int(self.model.predict([np.array(newsample)])[0]) #update our prediction
            if self.prediction==1:
                self.modelStatus.set("Level Ground")
            elif self.prediction==2:
                self.modelStatus.set("Down Stairs")
            else:
                self.modelStatus.set("Up Stairs")
    
    def deleteModel(self): 
        try: #if the model exists, delete it and reset everything
            remove(self.modelName)
            self.model=[]
            self.modelExists=0
            self.optimizedscore=-1
            self.modelStatus.set("Model Inactive")
            self.database=[]
            self.levelSteps=0
            self.descendSteps=0
            self.ascendSteps=0
            self.levelStepsLabel.set(str(self.levelSteps))
            self.descendStepsLabel.set(str(self.descendSteps))
            self.ascendStepsLabel.set(str(self.ascendSteps))
        except:
            pass
        
            
