//
//  Shader.fsh
//  Sub Mare Imperium: Derelict (GL ES)
//
//  Created by Daniel Monteiro on 12/29/23.
//  Copyright (c) 2023 Daniel Monteiro. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
