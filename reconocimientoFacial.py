
from firebase import firebase
#from collections.abc import MutableMapping
import face_recognition
from os import remove 
from os import listdir
import os

firebase = firebase.FirebaseApplication("https://sca-esp-default-rtdb.europe-west1.firebasedatabase.app/",None)
pathCompare = "C:/Users/Alberto/Documents/DataWifi/Compare"

firebase.put('/Rostros/id1','recog',3)

'''while(1):


    dir = os.listdir(pathCompare)
    if(len(dir) > 0):
    
        os.system('cd /Users/Alberto/Documents/DataWifi/Compare ; chmod 775 *')
    
        name = str(dir)
        separar = name.split('.')[0]
        var = str(separar)
        number=var.split('_')[1]
        idNum=str(number)
    
    
        image = face_recognition.load_image_file('C:/Users/Alberto/Documents/DataWifi/id'+idNum+'.jpg')
        face_loc = face_recognition.face_locations(image)[0]
        face_image_encodings = face_recognition.face_encodings(image, known_face_locations=[face_loc])[0]

        nueva = face_recognition.load_image_file(pathCompare+'/comparar_'+idNum+'.jpg')
        face_locations = face_recognition.face_locations(nueva)
        if face_locations == []:
            print("No se detecta una cara")
            remove(pathCompare+'/comparar_'+idNum+'.jpg')
            firebase.put('/Rostros/id'+number,'recog',2)
            
        if face_locations != []:
            for face_location in face_locations:
                face_new_encondings = face_recognition.face_encodings(nueva,known_face_locations=[face_location])[0]
                result = face_recognition.compare_faces(face_image_encodings,[face_new_encondings])
                if result[0] == True:
                    print("Las caras coinciden")
                    remove(pathCompare+'/comparar_'+idNum+'.jpg')
                    firebase.put('/Rostros/id'+number,'recog',1)
                if result[0] == False:
                    print("Las caras NO coinciden")
                    remove(pathCompare+'/comparar_'+idNum+'.jpg')
                    firebase.put('/Rostros/id'+number,'recog',2)'''

