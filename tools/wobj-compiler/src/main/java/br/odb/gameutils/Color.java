package br.odb.gameutils;

import java.io.Serializable;

/**
 * Very agnostic (but somewhat biased towards OpenGL ES) colour class. Most operations will clamp channel values 
 * between 0 and 255, but consistency can't be guaranteed with direct access
 * 
 * @author Daniel "Monty" Monteiro
 */
public class Color implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = -3950308399852846336L;

	public int r;
	public int g;
	public int b;
	public int a;
	
	/**
	 * 
	 * @return as float the channels as a, r, b, a
	 */
	public float[] getFloatData() {
		return new float[] { r / 255.0f, g / 255.0f, b / 255.0f , a / 255.0f };
	}
	
	/**
	 * @param argb
	 *            a 32-bit integer with a ARGB colour encoded.
	 */
	public Color(int argb) {
		set( (argb & 0x00FF0000) >> 16, (argb & 0x0000FF00) >> 8, ((argb & 0x000000FF)), argb >> 24 & 0xFF);
	}

	/**
	 * 
	 * @param r
	 *            integer Red (0-255)
	 * @param g
	 *            integer Green (0-255)
	 * @param b
	 *            integer Blue (0-255)
	 * @param a 
	 * 			  integer opacity ( 0 = invisible, 255 = fully visible )
	 */
	public Color(int r, int g, int b, int a) {
		set(r, g, b, a);
	}

	/**
	 * Initialized the colour as a full-opaque black
	 */
	public Color() {
		this(0, 0, 0, 255);
	}

	/**
	 * Initialize the colour in full-opaque, with the parameters
	 * 
	 * @param r
	 *            integer Red (0-255)
	 * @param g
	 *            integer Green (0-255)
	 * @param b
	 *            integer Blue (0-255)
	 */
	public Color(int r, int g, int b) {
		this(r, g, b, 255);
	}

	/**
	 * Copy constructor
	 * 
	 * @param c
	 *            The original colour
	 */
	public Color(Color c) {
		this(c.r, c.g, c.b, c.a);
	}

	/**
	 * Float constructor
	 * @param r (0.0-1.0) Red
	 * @param g (0.0-1.0) Green
	 * @param b (0.0-1.0) Blue
	 */
	public Color(float r, float g, float b) {
		set( (int) (r * 256), (int) (g * 256), (int) (b * 256), 255 );
	}

	/**
	 * Float constructor
	 * @param r (0.0-1.0) Red
	 * @param g (0.0-1.0) Green
	 * @param b (0.0-1.0) Blue
	 * @param a (0.0-1.0) Alpha
	 */	
	public Color(float r, float g, float b, float a) {
		set( (int) (r * 256), (int) (g * 256), (int) (b * 256), ( int )( a * 256 ) );
	}

	@Override
	public String toString() {

		StringBuffer sb = new StringBuffer();
		String hex;

		hex = Integer.toString(r);
		sb.append(hex);
		sb.append(" ");

		hex = Integer.toString(g);
		sb.append(hex);
		sb.append(" ");

		hex = Integer.toString(b);
		sb.append(hex);
		sb.append(" ");

		hex = Integer.toString(a);
		sb.append(hex);

		return sb.toString();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + a;
		result = prime * result + b;
		result = prime * result + g;
		result = prime * result + r;
		return result;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof Color)) {
			return false;
		}
		Color other = (Color) obj;
		if (a != other.a) {
			return false;
		}
		if (b != other.b) {
			return false;
		}
		if (g != other.g) {
			return false;
		}

		return r == other.r;
	}

	public int getARGBColor() {

		int color = 0;
		color = (a & 0xFF) << 24;
		color += (r & 0xFF) << 16;
		color += (g & 0xFF) << 8;
		color += b;

		return color;
	}

	public void set(int r, int g, int b, int a) {
		this.a = Utils.clamp( a, 0, 255 );
		this.r = Utils.clamp( r, 0, 255 );
		this.g = Utils.clamp( g, 0, 255 );
		this.b = Utils.clamp( b, 0, 255 );
	}

	public void set(int r, int g, int b) {
		set(r, g, b, 255);
	}

	public static Color getColorFromHTMLColor(String htmlColor) {
		Color toReturn = new Color();

		toReturn.r = (Integer.parseInt(htmlColor.substring(1, 3), 16));
		toReturn.g = (Integer.parseInt(htmlColor.substring(3, 5), 16));
		toReturn.b = (Integer.parseInt(htmlColor.substring(5, 7), 16));
		toReturn.a = (255);

		return toReturn;
	}

	public String getHTMLColor() {
		// 0x100 makes sure we have a 3 digit number, promptly culled with the
		// substring(1).
		String rHex = Integer.toString((r & 0xff) + 0x100, 16).substring(1);
		String gHex = Integer.toString((g & 0xff) + 0x100, 16).substring(1);
		String bHex = Integer.toString((b & 0xff) + 0x100, 16).substring(1);
		return "#" + rHex + gHex + bHex;
	}

	public void set(Color color) {
		
		if ( color == null ) {
			return;
		}
		
		set(color.r, color.g, color.b, color.a);
	}

	public String getExplicitRGBColor() {
		return "rgb( " + r + ", " + g + ", " + b + " )";
	}

	public void multiply(float factor) {
		set( (int) (r * factor), (int) (g * factor), (int) (b * factor), (int) (a * factor) );
	}
}
