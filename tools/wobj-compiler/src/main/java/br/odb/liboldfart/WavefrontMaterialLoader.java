package br.odb.liboldfart;

import br.odb.gameutils.Color;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

public class WavefrontMaterialLoader {

	private static final String NEW_MATERIAL_COMMAND = "newmtl";
	private static final String DIFFUSE_COLOR_COMMAND = "Kd";
	private static final char COMMENT_COMMAND = '#';
	private static final String TEXTURE_MAP_COMMAND = "map_Kd";

	private void parseLine(String line) {

		String[] subToken;
		String opcode;

		if (!isLineRelevant(line)) {
			return;
		}

		subToken = line.split("[ ]+");

		opcode = subToken[0];

		if (NEW_MATERIAL_COMMAND.equals(opcode)) {
			String op1 = subToken[1];
			currentMaterial = new WavefrontMaterial(op1);
			materials.add(currentMaterial);
		}

		if (DIFFUSE_COLOR_COMMAND.equals(opcode) && currentMaterial != null) {
			int r = parseIn255Range(subToken[1]);
			int g = parseIn255Range(subToken[2]);
			int b = parseIn255Range(subToken[3]);
			currentMaterial.addColor( new Color(r, g, b) );
		}

		if (TEXTURE_MAP_COMMAND.equals(opcode) && currentMaterial != null) {
			currentMaterial.addTexture( subToken[1] );
		}

	}

	private boolean isLineRelevant(String line) {
		return line != null && line.length() > 0 && line.charAt(0) != COMMENT_COMMAND;
	}

	private int parseIn255Range(String s) {
		return (int) (255 * Float.parseFloat(s));
	}

	public List<WavefrontMaterial> parseMaterials(InputStream is) {

		if ( is != null ) {
			Scanner in = new Scanner( is );

			while (in.hasNextLine()) {
				parseLine(in.nextLine());
			}
		}

		return materials;
	}

	private WavefrontMaterial currentMaterial = null;
	private final List<WavefrontMaterial> materials = new ArrayList<>();
}
