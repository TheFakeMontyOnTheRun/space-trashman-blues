package br.odb.liboldfart;

import br.odb.gameutils.Color;
import br.odb.gameutils.math.Vec2;
import br.odb.gameutils.math.Vec3;
import br.odb.libstrip.GeneralTriangle;
import br.odb.libstrip.Material;
import br.odb.libstrip.TriangleMesh;
import br.odb.libstrip.builders.GeneralTriangleFactory;

import java.io.InputStream;
import java.util.*;

public class SimpleWavefrontOBJLoader {

	private static final String USE_MATERIAL_COMMAND = "usemtl";
	private static final String NEW_VERTEX_COMMAND = "v";
	private static final String NEW_UVCOORD_COMMAND = "vt";
	private static final String NEW_OBJECT_COMMAND = "o";
	private static final String NEW_FACE_COMMAND = "f";
	private static final char COMMENT_COMMAND = '#';

	private interface WavefrontCommandProcessor {
		void run(String[] tokens);
	}

	public SimpleWavefrontOBJLoader(final GeneralTriangleFactory factory) {

		processorMap.put(USE_MATERIAL_COMMAND, new WavefrontCommandProcessor() {
			@Override
			public void run(String[] subToken) {
				if (materials.containsKey(subToken[1])) {
					currentMaterial = materials.get(subToken[1]);
				} else {
					currentMaterial = NEUTRAL_MATERIAL;
				}
			}
		});

		processorMap.put(NEW_VERTEX_COMMAND, new WavefrontCommandProcessor() {
			@Override
			public void run(String[] subToken) {
				Vec3 v = new Vec3(Float.parseFloat(subToken[1]),
						Float.parseFloat(subToken[2]),
						Float.parseFloat(subToken[3]));

				vertexes.add(v);
			}
		});

		processorMap.put(NEW_UVCOORD_COMMAND, new WavefrontCommandProcessor() {
			@Override
			public void run(String[] subToken) {
				Vec2 v = new Vec2(Float.parseFloat(subToken[1]),
						Float.parseFloat(subToken[2]));
				uvCoords.add(v);
			}
		});

		processorMap.put(NEW_OBJECT_COMMAND, new WavefrontCommandProcessor() {
			@Override
			public void run(String[] tokens) {
				currentMesh = new TriangleMesh(tokens[1]);
				meshList.add(currentMesh);
			}
		});

		processorMap.put(NEW_FACE_COMMAND, new WavefrontCommandProcessor() {
			@Override
			public void run(String[] subToken) {

				List<Vec3> temporary = new ArrayList<>();
				List<Vec2> temporaryUV = new ArrayList<>();
				List<Integer> indices = new ArrayList<>();

				for (int c = 1; c < subToken.length; ++c) {
					String substr = subToken[c];

					if (substr.contains("/")) {
						substr = substr.substring(0, substr.indexOf('/'));
					}

					indices.add(Integer.parseInt(substr) - 1);
					temporary.add(vertexes.get(Integer.parseInt(substr) - 1));

					substr = subToken[c];

					if (substr.contains("/")) {
						substr = substr.substring(substr.indexOf('/') + 1);
					} else {
						continue;
					}

					if (substr.contains("/")) {
						substr = substr.substring(0, substr.indexOf('/'));
					}

					temporaryUV.add(uvCoords.get(Integer.parseInt(substr) - 1));
				}

				GeneralTriangle triangle = factory.makeTrig(
						temporary.get(0).x, temporary.get(0).y, temporary.get(0).z,
						temporary.get(1).x, temporary.get(1).y, temporary.get(1).z,
						temporary.get(2).x, temporary.get(2).y, temporary.get(2).z,
						currentMaterial);

				float[] uv = new float[6];
				int i = 0;
				for (Vec2 v: temporaryUV) {
					uv[i++] = v.x;
					uv[i++] = v.y;
				}

				triangle.setTextureCoordinates(uv);
                                
				for (int c = 0; c < indices.size(); ++c) {
                                    triangle.indices[c] = indices.get(c);
				}                                

				currentMesh.faces.add(triangle);
			}
		});
	}


	private void parseLine(String line) {

		if (!isRelevant(line)) {
			return;
		}

		String[] subToken = line.split("[ ]+");
		WavefrontCommandProcessor processor = processorMap.get(subToken[0]);

		if (processor != null) {
			processor.run(subToken);
		}
	}

	private boolean isRelevant(String line) {
		return line != null && line.length() > 0 && line.charAt(0) != COMMENT_COMMAND;
	}

	private void resetLoader() {
		materials.clear();
		meshList.clear();
	}

	public List<TriangleMesh> loadMeshes(InputStream is, List<WavefrontMaterial> materialList) {

		resetLoader();

		if (materialList != null) {
			for (WavefrontMaterial m : materialList) {
				materials.put(m.name, m.material);
			}
			currentMaterial = null;
		}

		meshList.add(currentMesh = new TriangleMesh(""));

		if (is != null) {
			Scanner in = new Scanner(is);

			while (in.hasNextLine()) {
				parseLine(in.nextLine());
			}
		}

		clearEmptyMeshes(meshList);

		return meshList;
	}

	private void clearEmptyMeshes(List<TriangleMesh> meshList) {
		List<TriangleMesh> toRemove = new ArrayList<>();

		for (TriangleMesh mesh : meshList) {
			if (mesh.faces.size() == 0) {
				toRemove.add(mesh);
			}
		}

		meshList.removeAll(toRemove);
	}

	final private List<TriangleMesh> meshList = new ArrayList<>();
	final private Map<String, Material> materials = new HashMap<>();
	final public List<Vec3> vertexes = new ArrayList<>();
	final public List<Vec2> uvCoords = new ArrayList<>();
	private TriangleMesh currentMesh;
	private Material currentMaterial;
	private final Map<String, WavefrontCommandProcessor> processorMap = new HashMap<>();
	private static final Material NEUTRAL_MATERIAL = Material.makeWithColor(new Color(0xFFFFFFFF));
}
