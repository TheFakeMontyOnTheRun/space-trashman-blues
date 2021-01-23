using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//[ExecuteInEditMode]
public class Exportable : MonoBehaviour
{
    public float ceilingHeight;
    public float floorHeight;
    public float floorRepetitions;
    public float ceilingRepetitions;
    public Material mainMaterial;
    public Material ceilingMaterial;
    public Material ceilingRepetitionsMaterial;
    public Material floorMaterial;
    public Material floorRepetitionsMaterial;

    public bool blockVisibility;
    public bool blockMovement;
    public bool needsAlphaTest;
    public string representation;


    void Update()
    {

    }
}
