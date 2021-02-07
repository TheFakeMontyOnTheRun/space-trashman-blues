using System.Collections;
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
    }

    public override void OnInspectorGUI() {
        serializedObject.Update();

        EditorGUILayout.PropertyField(ceilingHeight);
        EditorGUILayout.PropertyField(floorHeight);
        EditorGUILayout.PropertyField(floorRepetitions);
        EditorGUILayout.PropertyField(ceilingRepetitions);
        EditorGUILayout.PropertyField(mainMaterial);
        EditorGUILayout.PropertyField(ceilingMaterial);
        EditorGUILayout.PropertyField(ceilingRepetitionsMaterial);
        EditorGUILayout.PropertyField(floorMaterial);
        EditorGUILayout.PropertyField(floorRepetitionsMaterial);

        if (GUILayout.Button("Apply")) {
            var tempList = (target as Exportable).transform.Cast<Transform>().ToList();
            foreach(Transform child in tempList) {
                DestroyImmediate(child.gameObject);
            }

            (target as Exportable).Apply();

        }
        
        serializedObject.ApplyModifiedProperties();
        
    }
    
}
