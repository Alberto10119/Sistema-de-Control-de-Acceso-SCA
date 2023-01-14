
from firebase import firebase
import cv2
import face_recognition
from os import remove 
from os import listdir
import os
from pathlib import Path



firebase = firebase.FirebaseApplication("https://sca-esp-default-rtdb.europe-west1.firebasedatabase.app/",None)
pathID = "C:/Users/Alberto/Documents/DataWifi"

x=1

while(1):

    dir = os.listdir(pathID)
    for x in range(1,len(dir)+1):

        num = str(x)    
        fileName = r"C:/Users/Alberto/Documents/DataWifi/id"+num+".jpg"
        file = Path(fileName)

        if(file.exists()):
        
            image = face_recognition.load_image_file('C:/Users/Alberto/Documents/DataWifi/id'+num+'.jpg')
            face_loc = face_recognition.face_locations(image)
            if face_loc == []:
                print("No se detecta una cara")
                firebase.put('/Rostros/id'+num,'recog',3)
                remove(file)
            elif face_loc != []:
                leer = firebase.get('Rostros/id'+num,'recog')
                if (leer == 4):
                    firebase.put('/Rostros/id'+num,'recog',0)
                    print("Cara detectada")
    