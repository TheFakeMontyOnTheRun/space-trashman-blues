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

            if (floorMaterial.objectReferenceValue != null ) {
                GameObject floorGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                floorGO.transform.parent  = (target as Exportable).transform;
                floorGO.transform.localPosition = new Vector3(0, floorHeight.floatValue, 0);
                floorGO.transform.localScale = new Vector3( 1, 0, 1 );
                floorGO.name = "floor";
                floorGO.GetComponent<MeshRenderer>().material = floorMaterial.objectReferenceValue as Material;
            }


            if (ceilingMaterial.objectReferenceValue) {
                GameObject ceilingGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingGO.transform.parent  = (target as Exportable).transform; 
                ceilingGO.transform.localPosition = new Vector3(0, ceilingHeight.floatValue, 0);
                ceilingGO.transform.localScale = new Vector3( 1, 0, 1 );
                ceilingGO.name = "ceiling";
                ceilingGO.GetComponent<MeshRenderer>().material = ceilingMaterial.objectReferenceValue as Material;
            }

            if (ceilingRepetitions.floatValue > 0 && ceilingRepetitionsMaterial.objectReferenceValue != null ) {
                GameObject ceilingRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                ceilingRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                ceilingRepetitionsGO.transform.localPosition = new Vector3(0, ceilingHeight.floatValue + (ceilingRepetitions.floatValue / 2), 0);
                ceilingRepetitionsGO.transform.localScale = new Vector3( 1, ceilingRepetitions.floatValue, 1 );
                ceilingRepetitionsGO.name = "ceilingRepetition";
                ceilingRepetitionsGO.GetComponent<MeshRenderer>().material = ceilingRepetitionsMaterial.objectReferenceValue as Material;
            }

            if (floorRepetitions.floatValue > 0 && floorRepetitionsMaterial.objectReferenceValue != null) {
                GameObject floorRepetitionsGO = GameObject.CreatePrimitive(PrimitiveType.Cube);
                floorRepetitionsGO.transform.parent  = (target as Exportable).transform; 
                floorRepetitionsGO.transform.localPosition = new Vector3(0, floorHeight.floatValue - (floorRepetitions.floatValue / 2), 0);
                floorRepetitionsGO.transform.localScale = new Vector3( 1, floorRepetitions.floatValue, 1 );
                floorRepetitionsGO.name = "floorRepetition";
                floorRepetitionsGO.GetComponent<MeshRenderer>().material = floorRepetitionsMaterial.objectReferenceValue as Material;
            }

        }
        
        serializedObject.ApplyModifiedProperties();
        
    }
    
}
