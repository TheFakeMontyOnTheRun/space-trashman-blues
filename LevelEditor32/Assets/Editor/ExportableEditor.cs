using System.Collections;
﻿using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Linq;

[CustomEditor(typeof(Exportable))]
[CanEditMultipleObjects]
public class ExportableEditor : Editor
{
    SerializedProperty ceilingHeight;
    SerializedProperty floorHeight;
    SerializedProperty floorRepetitions;
    SerializedProperty ceilingRepetitions;
    SerializedProperty mainMaterial;
    SerializedProperty ceilingMaterial;
    SerializedProperty ceilingRepetitionsMaterial;
    SerializedProperty floorMaterial;
    SerializedProperty floorRepetitionsMaterial;
    SerializedProperty representation;
    SerializedProperty geometryType;

    SerializedProperty repeatMainTexture;

    void OnEnable() {
        ceilingHeight = serializedObject.FindProperty("ceilingHeight");
        floorHeight = serializedObject.FindProperty("floorHeight");
        floorRepetitions = serializedObject.FindProperty("floorRepetitions");
        ceilingRepetitions = serializedObject.FindProperty("ceilingRepetitions");
        mainMaterial = serializedObject.FindProperty("mainMaterial");        
        ceilingMaterial = serializedObject.FindProperty("ceilingMaterial");
        ceilingRepetitionsMaterial = serializedObject.FindProperty("ceilingRepetitionsMaterial");
        floorMaterial = serializedObject.FindProperty("floorMaterial");
        floorRepetitionsMaterial = serializedObject.FindProperty("floorRepetitionsMaterial");
        representation = serializedObject.FindProperty("representation");
        repeatMainTexture = serializedObject.FindProperty("repeatMainTexture");
        geometryType = serializedObject.FindProperty("geometryType");
    }

    public override void OnInspectorGUI() {
        serializedObject.Update();

        if (String.IsNullOrEmpty(representation.stringValue.Trim())) {
            representation.stringValue = "1";
        }

        EditorGUILayout.PropertyField(ceilingHeight);
        EditorGUILayout.PropertyField(floorHeight);
        EditorGUILayout.PropertyField(floorRepetitions);
        EditorGUILayout.PropertyField(ceilingRepetitions);
        EditorGUILayout.PropertyField(mainMaterial);
        EditorGUILayout.PropertyField(ceilingMaterial);
        EditorGUILayout.PropertyField(ceilingRepetitionsMaterial);
        EditorGUILayout.PropertyField(floorMaterial);
        EditorGUILayout.PropertyField(floorRepetitionsMaterial);
        EditorGUILayout.PropertyField(representation);
        EditorGUILayout.PropertyField(repeatMainTexture);
        EditorGUILayout.PropertyField(geometryType);

        if (GUILayout.Button("Apply")) {
            var tempList = (target as Exportable).transform.Cast<Transform>().ToList();
            foreach(Transform child in tempList) {
                DestroyImmediate(child.gameObject);
            }

            (target as Exportable).Apply();
        }

    
        if (GUILayout.Button("Copy FROM representation")) {
            if (Exportable.GeneralTable.ContainsKey(representation.stringValue))
            {
                (target as Exportable).CopyFrom(Exportable.GeneralTable[representation.stringValue]);
            }
        }        

        if (GUILayout.Button("Copy TO representation")) {
            Exportable.GeneralTable[representation.stringValue] = (target as Exportable); 
        }

        if (GUILayout.Button("Apply globally *CAREFUL*")) {
            Exportable.GeneralTable[representation.stringValue] = (target as Exportable); 
            var geometryRoot = GameObject.Find("Geometry");
            List<GameObject> children = new List<GameObject>();

            foreach (GameObject child  in FindObjectsOfType(typeof(GameObject)) ) {

                if (child.GetComponent<Exportable>() && child.GetComponent<Exportable>().representation.Equals(representation.stringValue)) {
                    children.Add(child);
                }
            }

            foreach( GameObject child in children ) {
                var tempList = child.transform.Cast<Transform>().ToList();
                foreach(Transform undesirable in tempList) {
                    DestroyImmediate(undesirable.gameObject);
                }
                child.GetComponent<Exportable>().CopyFrom((target as Exportable));                    
                child.GetComponent<Exportable>().Apply();
            }
        }

        if (GUILayout.Button("Find next available index"))
        {
            for (int c = 46; c < 255; ++c) {
                if (!Exportable.GeneralTable.ContainsKey("" + c )) {
                    representation.stringValue = "" + c;
                    serializedObject.ApplyModifiedProperties();
                    return;
                }
            }
        }

        serializedObject.ApplyModifiedProperties();
    }
}
