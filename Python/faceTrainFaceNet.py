import os
import cv2
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from facenet_pytorch import MTCNN, InceptionResnetV1
import pickle

# Define the directory containing the images
train_dir = 'train_dir/' # put yor train_dir path like 'C:/Users/username/train_dir/'
miss_count = 0
# Initialize the MTCNN module
mtcnn = MTCNN()

# Initialize a pre-trained InceptionResnetV1 model
resnet = InceptionResnetV1(pretrained='vggface2').eval()

# Prepare the data
X = []
y = []
for person_id in os.listdir(train_dir):
    person_dir = os.path.join(train_dir, person_id)
    if os.path.isdir(person_dir):
        for image_name in os.listdir(person_dir):
            image_path = os.path.join(person_dir, image_name)
            img = cv2.imread(image_path)
            img_cropped = mtcnn(img)
            if img_cropped is not None:  # Add this line
                img_embedding = resnet(img_cropped.unsqueeze(0))
                X.append(img_embedding.detach().numpy().reshape(-1))  # Reshape the embeddings
                y.append(person_id)
            else:
                miss_count+=1
X = np.array(X)
y = np.array(y)

# Split the data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train a SVM classification model
model = SVC(kernel='linear', probability=True)
model.fit(X_train, y_train)

# Save the model
with open('svm_model.pkl', 'wb') as f:
    pickle.dump(model, f)
print(miss_count)
