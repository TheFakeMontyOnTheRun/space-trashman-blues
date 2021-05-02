using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;
using System;

public class Exports : MonoBehaviour
{
    public class PetEntry
    {
        public float ceilingHeight;
        public float floorHeight;
        public float floorRepetitions;
        public float ceilingRepetitions;
        public string mainMaterial;
        public string ceilingMaterial;
        public string ceilingRepetitionsMaterial;
        public string floorMaterial;
        public string floorRepetitionsMaterial;

        public bool blockVisibility;
        public bool blockMovement;
        public bool repeatMainTexture;
        public bool needsAlphaTest;
        public bool blockEnemySight;
        public string representation;
        public Exportable.GeometryType geometryType;
    }

    [MenuItem("Monty/Reset scene as 64x64")]
    static void ResetLevel64()
    {
        ResetLevel(64);
    }

    [MenuItem("Monty/Reset scene as 32x32")]
    static void ResetLevel32()
    {
        ResetLevel(32);
    }


    static void ResetLevel(int size)
    {
        var objects = FindObjectsOfType(typeof(GameObject));

        foreach (GameObject child in objects)
        {
            DestroyImmediate(child.gameObject);
        }

        GameObject geometryRoot = new GameObject("Geometry");
        GameObject spawner;
        Material matRef1 = AssetDatabase.LoadAssetAtPath("Assets/Materials/asphalt.mat", typeof(Material)) as Material;
        Material matRef2 = AssetDatabase.LoadAssetAtPath("Assets/Materials/ceiling.mat", typeof(Material)) as Material;
        matRef1.mainTexture.filterMode = FilterMode.Point;
        matRef2.mainTexture.filterMode = FilterMode.Point;

        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                spawner = new GameObject("tile" + x + "_" + y);
                spawner.transform.parent = geometryRoot.transform;
                spawner.transform.position = new Vector3(x, 0, y);


                spawner.AddComponent<Exportable>();
                spawner.GetComponent<Exportable>().floorMaterial = matRef1;
                spawner.GetComponent<Exportable>().ceilingMaterial = matRef2;
                spawner.GetComponent<Exportable>().ceilingHeight = 1.0f;
                spawner.GetComponent<Exportable>().representation = "46";
                spawner.GetComponent<Exportable>().Apply();
            }
        }

        GameObject playerSpawner = new GameObject("PlayerSpawner");
        playerSpawner.AddComponent<PlayerSpawner>();
        playerSpawner.transform.position = new Vector3(1, 1, 1);
    }


    [MenuItem("Monty/Export 64x64 Level as Data File")]
    static void Export64Level()
    {
        ExportLevelFile(64);
    }

    [MenuItem("Monty/Export 32x32 Level as Data File")]
    static void Export32Level()
    {
        ExportLevelFile(32);
    }

    static void ExportLevelFile(int size) { 
        GameObject[] objects = GameObject.FindObjectsOfType(typeof(GameObject)) as GameObject[];

        int[,] map = new int[size, size];
        Exportable[] pet = new Exportable[256];

        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                map[y, x] = 46;
            }
        }

        foreach (GameObject go in objects)
        {
            if (go.activeInHierarchy)
            {
                var exportbl = go.GetComponent(typeof(Exportable)) as Exportable;
                if (exportbl != null)
                {
                    int x = (int)(go.transform.position.x);
                    int z = (size - 1) - (int)(go.transform.position.z);

                    if (z < size && x < size && x >= 0 && z >= 0 ) {
                        map[z, x] = Convert.ToInt32(exportbl.representation[0]);
                        pet[map[z, x]] = exportbl;
                    }
                }
            }
        }

        string finalMap = "";

        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                finalMap += Convert.ToChar(map[y, x]);
            }

            finalMap += "\n";
        }
        

        string petCode = "";

        for (int c = 0; c < 256; ++c)
        {

            if (pet[c] == null)
            {
                continue;
            }

            petCode += Convert.ToChar(c);
            petCode += " ";
            petCode += pet[c].needsAlphaTest ? 1 : 0;
            petCode += " ";
            petCode += pet[c].blockVisibility ? 1 : 0;
            petCode += " ";
            petCode += pet[c].blockMovement ? 1 : 0;
            petCode += " ";
            petCode += "0"; //blocks enemy sight
            petCode += " ";
            petCode += pet[c].repeatMainTexture ? 1 : 0;
            petCode += " ";
            petCode += pet[c].ceilingMaterial != null ? pet[c].ceilingMaterial.name : "null";
            petCode += " ";
            petCode += pet[c].floorMaterial != null ? pet[c].floorMaterial.name : "null";
            petCode += " ";
            petCode += pet[c].mainMaterial != null ? pet[c].mainMaterial.name : "null";
            petCode += " ";
            petCode += pet[c].geometryType != Exportable.GeometryType.None ? pet[c].geometryType.ToString().ToLower() : "null";
            petCode += " ";
            petCode += pet[c].ceilingRepetitionsMaterial != null ? pet[c].ceilingRepetitionsMaterial.name : "null";
            petCode += " ";
            petCode += pet[c].floorRepetitionsMaterial != null ? pet[c].floorRepetitionsMaterial.name : "null";
            petCode += " ";
            petCode += pet[c].ceilingRepetitions;
            petCode += " ";
            petCode += pet[c].floorRepetitions;
            petCode += " ";
            petCode += pet[c].ceilingHeight;
            petCode += " ";
            petCode += pet[c].floorHeight;

            petCode += "\n";
        }

        petCode += "";

        string path = "Assets/Map.txt";
        StreamWriter writer = new StreamWriter(path, false);
        writer.WriteLine(finalMap.Trim());
        writer.Close();

        AssetDatabase.ImportAsset(path);


        path = "Assets/Tiles.prp";
        writer = new StreamWriter(path, false);
        writer.WriteLine(petCode.Trim());
        writer.Close();

        AssetDatabase.ImportAsset(path);
    }

    [MenuItem("Monty/Update Materials")]
    static void UpdateMaterials()
    {
        var shader = Shader.Find("Unlit/Texture");
        var texturesInfo = new DirectoryInfo("Assets/Resources/Textures");
        var filesTextureInfo = texturesInfo.GetFiles();
        var materialsInfo = new DirectoryInfo("Assets/Materials/");
        var filesMaterialsInfo = materialsInfo.GetFiles();

        var materialList = "";

        foreach (var file in filesMaterialsInfo)
        {
            materialList += file.Name + ";";
        }

        foreach (var file in filesTextureInfo) {
            if (file.Name.EndsWith(".png") && !materialList.Contains(file.Name.Replace(".png", ""))) {
                var texture = Resources.Load("Textures/"  + file.Name.Replace(".png", ""), typeof(Texture2D)) as Texture2D;
                texture.filterMode = FilterMode.Point;
                var newMat = new Material(shader);
                newMat.mainTexture = texture;
                AssetDatabase.CreateAsset( newMat, "Assets/Materials/" + file.Name.Replace(".png", ".mat"));
            }
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();
    }

    static void ImportLevel(int size)
    {
        Dictionary<char, PetEntry> petTable = new Dictionary<char, PetEntry>();
        char[,] map = new char[size, size];
        StreamReader reader = new StreamReader("Assets/Tiles.prp");

        while (!reader.EndOfStream)
        {
            string line = reader.ReadLine();
            string[] tokens = line.Split(' ');
            char lead = tokens[0][0];
            PetEntry entry = new PetEntry();
            entry.needsAlphaTest = (tokens[1][0] == '1');
            entry.blockVisibility = (tokens[2][0] == '1');
            entry.blockMovement = (tokens[3][0] == '1');
            entry.blockEnemySight = (tokens[4][0] == '1');
            entry.repeatMainTexture = (tokens[5][0] == '1');

            entry.ceilingMaterial = tokens[6];
            entry.floorMaterial = tokens[7];
            entry.mainMaterial = tokens[8];

            if (!tokens[9].Equals("null")) {
                //not proud...
                foreach (Exportable.GeometryType geoType in Enum.GetValues(typeof(Exportable.GeometryType))) {
                    if (geoType.ToString().ToLower().Equals(tokens[9])) {
                        entry.geometryType = geoType;
                    }
                }
            }


            entry.ceilingRepetitionsMaterial = tokens[10];
            entry.floorRepetitionsMaterial = tokens[11];
            entry.ceilingRepetitions = Convert.ToInt32(tokens[12]);
            entry.floorRepetitions = Convert.ToInt32(tokens[13]);
            entry.ceilingHeight = Convert.ToSingle(tokens[14]);
            entry.floorHeight = Convert.ToSingle(tokens[15]);
            petTable[lead] = entry;
        }


        reader = new StreamReader("Assets/Map.txt");
        for (int y = 0; y < size; ++y)
        {
            string line = reader.ReadLine();
            for (int x = 0; x < size; ++x)
            {
                char entry = line[x];
                map[y, x] = entry;

            }
        }

        var objects = FindObjectsOfType(typeof(GameObject));

        foreach (GameObject child in objects)
        {
            DestroyImmediate(child.gameObject);
        }

        GameObject geometryRoot = new GameObject("Geometry");
        GameObject spawner;

        for (int _y = 0; _y < size; ++_y)
        {
            int y = _y;
            for (int _x = 0; _x < size; ++_x)
            {
                int x = size - _x - 1;
                spawner = new GameObject("tile" + x + "_" + y);
                spawner.transform.parent = geometryRoot.transform;
                spawner.transform.position = new Vector3(_x, 0, _y);


                spawner.AddComponent<Exportable>();
                spawner.GetComponent<Exportable>().representation = "" + map[y, x];

                if (Exportable.GeneralTable.ContainsKey("" + map[y, x])) {
                    spawner.GetComponent<Exportable>().CopyFrom(Exportable.GeneralTable[spawner.GetComponent<Exportable>().representation]);
                } else {
                    var pet = petTable[map[y, x]];
                    var exportable = spawner.GetComponent<Exportable>();

                    exportable.blockMovement = pet.blockMovement;
                    exportable.blockVisibility = pet.blockVisibility;
                    exportable.ceilingMaterial = getMaterialRef(pet.ceilingMaterial);
                    exportable.floorMaterial = getMaterialRef(pet.floorMaterial);
                    exportable.mainMaterial = getMaterialRef(pet.mainMaterial);
                    exportable.geometryType = pet.geometryType;
                    exportable.ceilingRepetitionsMaterial = getMaterialRef(pet.ceilingRepetitionsMaterial);
                    exportable.floorRepetitionsMaterial = getMaterialRef(pet.floorRepetitionsMaterial);
                    exportable.ceilingRepetitions = pet.ceilingRepetitions;
                    exportable.floorRepetitions = pet.floorRepetitions;
                    exportable.floorHeight = pet.floorHeight;
                    exportable.ceilingHeight = pet.ceilingHeight;

                    Exportable.GeneralTable["" + map[y, x]] = exportable;
                }
                spawner.GetComponent<Exportable>().Apply();
            }
        }

        GameObject playerSpawner = new GameObject("PlayerSpawner");
        playerSpawner.AddComponent<PlayerSpawner>();
        playerSpawner.transform.position = new Vector3(1, 1, 1);


        reader.Close();
        ReacquireAllPatterns();
    }

    [MenuItem("Monty/Import 64x64 level")]
    static void Import64Level()
    {
        ImportLevel(64);
    }

    static Material getMaterialRef(string name) {
        return AssetDatabase.LoadAssetAtPath(String.Format("Assets/Materials/{0}.mat", name), typeof(Material)) as Material;
    }

    [MenuItem("Monty/Import 32x32 level")]
    static void Import32Level()
    {
        ImportLevel(32);
    }



    [MenuItem("Monty/Reacquire all patterns")]
    static void ReacquireAllPatterns()
    {
        Exportable.GeneralTable.Clear();

        GameObject[] objects = GameObject.FindObjectsOfType(typeof(GameObject)) as GameObject[];

        foreach (GameObject go in objects)
        {
            if (go.activeInHierarchy)
            {
                var exportbl = go.GetComponent(typeof(Exportable)) as Exportable;
                if (exportbl != null)
                {
                    Exportable.GeneralTable[exportbl.representation] = exportbl;
                }
            }
        }
    }

    /*
        const int kMapSize = 32;

        [MenuItem("Monty/Export Level as C Source File")]
        static void ExportLevelAsSource() {
            Debug.Log("Doing stuff in C");

            GameObject[] objects = GameObject.FindObjectsOfType(typeof(GameObject)) as GameObject[];

            string[,] map = new string[kMapSize, kMapSize];
            PetEntry[] pet = new PetEntry[256];

            for (int y = 0; y < kMapSize; ++y) {
                for ( int x = 0; x < kMapSize; ++x ) {
                    map[y, x] = "0";
                }
            }

            foreach (GameObject go in objects) {
                if (go.activeInHierarchy) {
                    var exportbl = go.GetComponent(typeof(Exportable)) as Exportable;
                    if (exportbl != null) {
                        int x = (int) (go.transform.position.x );
                        int z = (kMapSize - 1) - (int) (go.transform.position.z );
                        print(go + ": " + go.name + " is active in hierarchy at ( " + x + ", " + z + " ) and is " + exportbl.representation);

                        if (z < kMapSize && x < kMapSize && x >= 0 && z >= 0)
                        {
                            map[z, x] = exportbl.representation;
                            pet[Int32.Parse(map[z, x])] = new PetEntry(0, (int)go.transform.localScale.x, 0, false, 0);
                        }
                    }
                }
            }


            for (int y = 1; y < kMapSize - 1; ++y)
            {
                for (int x = 1; x < kMapSize - 1; ++x)
                {
                    if (map[y, x] == "0" && (
                        (Int32.Parse(map[y - 1, x]) > 1) ||
                        (Int32.Parse(map[y + 1, x]) > 1) ||
                        (Int32.Parse(map[y, x + 1]) > 1) ||
                        (Int32.Parse(map[y, x - 1]) > 1) ||
                        (Int32.Parse(map[y - 1, x - 1]) > 1) ||
                        (Int32.Parse(map[y - 1, x + 1]) > 1) ||
                        (Int32.Parse(map[y + 1, x - 1]) > 1) ||
                        (Int32.Parse(map[y + 1, x + 1]) > 1))) {

                        map[y, x] = "1";
                    }
                }
            }

            string finalMap = "{\n";

            for (int y = 0; y < kMapSize; ++y)
            {
                finalMap += "{ ";
                for (int x = 0; x < kMapSize; ++x)
                {
                    finalMap += map[y, x] + ", ";
                }
                finalMap += "},\n";
            }
            finalMap += "};\n";

            string petCode = "patterns[16]={\n";

            for (int c = 0; c < 256; ++c ) {

                if (pet[c] == null) {
                    continue;
                }

                petCode += "{";
                petCode += pet[c].floor;
                petCode += ", ";
                petCode += pet[c].ceiling;
                petCode += ", ";
                petCode += pet[c].blockMovement;
                petCode += ", ";
                petCode += pet[c].geometryType;
                petCode += ", ";
                petCode += pet[c].textureIndex;
                petCode += " },\n";
            }

            petCode += "};";

            string path = "Assets/monty.txt";
            StreamWriter writer = new StreamWriter(path, false);
            writer.WriteLine(finalMap);
            writer.Close();

            AssetDatabase.ImportAsset(path);


            path = "Assets/pet.txt";
            writer = new StreamWriter(path, false);
            writer.WriteLine(petCode);
            writer.Close();

            AssetDatabase.ImportAsset(path);
        }
        */
}
