
import face_recognition
from os import remove 
from os import listdir
import os




pathCompare = "C:/Users/Alberto/Documents/DataWifi/Compare"

while(1):

    dir = os.listdir(pathCompare)
    if(len(dir) > 0):
    
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
            
        if face_locations != []:
            for face_location in face_locations:
                face_new_encondings = face_recognition.face_encodings(nueva,known_face_locations=[face_location])[0]
                result = face_recognition.compare_faces(face_image_encodings,[face_new_encondings])
                if result[0] == True:
                    
                    print("Los rostros coinciden")
                    remove(pathCompare+'/comparar_'+idNum+'.jpg')
                if result[0] == False:
                    print("Los rostros NO coinciden")
                    remove(pathCompare+'/comparar_'+idNum+'.jpg')
               


