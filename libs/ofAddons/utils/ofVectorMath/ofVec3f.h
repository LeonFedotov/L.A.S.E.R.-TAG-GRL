#ifndef _OF_VEC3f
#define _OF_VEC3f


class ofVec3f{

public:
	
	
    //----------------  Constructors
    
    
    /**
     * Constructor. Defaults to zero,zero,zero.
     *
     * @param  _x initial x coordinate.
     * @param  _y initial y coordinate.
     * @param  _z initial z coordinate.
     */
   
    ofVec3f( const float _x = 0.0f, const float _y = 0.0f, const float _z = 0.0f )
        { x = _x;  y = _y;  z = _z; }
	
    
    /**
     * Constructor. Copies value in vec.
     *
     * @param  vec source of initial coordinates to copy.
     */
    
    ofVec3f( const ofVec3f &vec )  
        { x = vec.x;  y = vec.y;  z = vec.z; }
    
    
    
    
    
    //---------------- Data Members
    
    // x-coordinate
    
    float x;
    
    // y-coordinate
    
    float y;
    
    // z-coordinate
    
    float z;
    
    
    
    //---------------- Methods
    
    /**
     * Set x,y,z coordinates of vector.
     *
     * @param  _x  x coordinate.
     * @param  _y  y coordinate.
     * @param  _z  z coordinate.
     */
    
    void set( 	float _x, float _y, float _z )
        { x = _x;  y = _y;  z = _z; }
    
    
    
    /**
     * Set x,y,z coordinates to the values in a vector object.
     *
     * @param vec  vector object to copy.
     */
     
     void set( const ofVec3f &vec )
       { x = vec.x;  y = vec.y;  z = vec.z; }

    
    
    /**
	  * Array access operator
	  * @param i Array index
	  * @return For i = 0, reference to x coordinate, i = 1
	  * reference to y, i = 2 reference to z else
	  * x coordinate.
	  */
    
    float &operator[]( const int &i )           
        { switch(i) {
          case 0:  return x;
          case 1:  return y;
          case 2:  return z;
          default: return x; } }

    
    
    /**
     * Addition operator.
     * @param vec the vector to add
     * @return this + vec
     */  
    
    ofVec3f operator+( const ofVec3f & vec ) const          
    { 
        return ofVec3f( x+vec.x, y+vec.y, z+vec.z); 
    } 
    
    
    
    /**
     * Add a vector into this
     * @param vec the vector to add
     * @return this 
     */
    
    ofVec3f &operator+=( const ofVec3f & vec )              
    { 
        x+=vec.x; y+=vec.y; z+=vec.z;
        return *this; 
    } 

    
    
    /**
     * Subtraction operator.
     * @return -this
     */ 
    
    ofVec3f operator-() const                               
    { 
        return ofVec3f(-x, -y, -z); 
    } 
    
    
    
    /**
     * Subtraction operator.
     * @param vec to subtract
     * @return this - vec
     */ 
    
    ofVec3f operator-( const ofVec3f & vec ) const          
    { 
        return ofVec3f(x-vec.x, y-vec.y, z-vec.z); 
    } 
    
    
    
    /**
     * Subtract a vector from this.
     * @param vec to subtract
     * @return this 
     */ 
    
    ofVec3f &operator-=( const ofVec3f & vec )              
    { 
        x-=vec.x; y-=vec.y; z-=vec.z;
        return *this; 
    } 
    
    
    
    /**
     * Multiplication operator.
     * @param vec to multiply by
     * @return this * vec
     */ 
    
    ofVec3f operator*( const ofVec3f & vec ) const
    { 
        return ofVec3f(x*vec.x, y*vec.y, z*vec.z); 
    } 
    
    
    
    /**
     * Multiplication operator.
     * @param vec to multiply by
     * @return this
     */ 
    
    ofVec3f &operator*=( const ofVec3f & vec )      
    { 
        x*=vec.x; y*=vec.y; z*=vec.z;
        return *this; 
    } 

    
    
    /**
     * Division operator.
     * @param vec to divide by
     * @return this / vec
     */ 
    
    ofVec3f operator/( const ofVec3f & vec ) const  
    { 
        return ofVec3f(x/vec.x, y/vec.y, z/vec.z);
    } 
    
    
    
    /**
     * Division operator.
     * @param vec to divide by
     * @return this
     */ 
    
    ofVec3f &operator/=( const ofVec3f & vec )      
    { 
        x/=vec.x; y/=vec.y; z/=vec.z;
        return *this; 
    } 

    
    
    /**
     * Scalar Multiplication. 
     * @param f value to multiply by. 
     * @return value of this scaled by value in f
     */ 
    
    ofVec3f operator*( const float f ) const                
    { 
        return ofVec3f(x*f, y*f, z*f);
    } 
    
      
    /**
     * Scalar Multiplication.
     * @param f value to multiply by 
     * @return this
     */ 
    
    ofVec3f &operator*=( const float f )                    
    { 
        x*=f; y*=f; z*=f;
        return *this; 
    } 

    /**
     * Scalar Multiplication.
     * @param f value to multiply by 
     * @param vec vector to scale
     * @return vector scaled by value in f
     */ 
    friend ofVec3f operator*( const float f, const ofVec3f &vec )  
    { 
        return ofVec3f(vec.x*f, vec.y*f, vec.z*f); 
    } 
    
    
    /**
     * Scalar Division. 
     * @param f value to divide by. 
     */ 
    
    ofVec3f operator/( const float f ) const      
    { 
        return ofVec3f(x/f, y/f, z/f);
    } 
    
    
    /**
     * Scalar Division. 
     * @param f value to divide by. 
     */ 
    
    ofVec3f &operator/=( const float f )          
    { 
        x/=f; y/=f; z/=f;
        return *this; 
    } 

    
    
    /**
     * Dot product of two vectors. 
     * @return dot product of this and vec
     */
    
    float dot( const ofVec3f & vec ) const        
    { 
        return x*vec.x + y*vec.y + z*vec.z; 
    } 
    
    
    
    /**
     * Cross product of two vectors. 
     * @return cross product of this and vec
    */
    
    ofVec3f cross( const ofVec3f & vec ) const      
    { 
        return ofVec3f(y*vec.z - z*vec.y, 
                       z*vec.x - x*vec.z, 
                       x*vec.y - y*vec.x); 
    } 


    
    
    /**
     * Calculate the length (magnitude) of the vector. 
     * @return length
    */
    
    float length() const                
    { 
        return (float)sqrt(x*x + y*y + z*z); 
    } 
    
    
    
    /**
     * Calculate the squared length of the vector. 
     * This should be faster than length()
     * @return length squared
    */
    
    float lengthSquared() const                
    { 
        return (float)(x*x + y*y + z*z); 
    } 
    
    
    /**
     * Normalizes this 
     * @return this
    */
    
    ofVec3f &normalize()                  
    { 
			float f = length();
			if( f > 0){ 
				x/=f; 
				y/=f; 
				z/=f;
			}
        
		  return *this; 
	} 
    
    
    
    /**
     * Calculate s normal without modifying this. 
     * @return normalized value
    */
    
    ofVec3f norm_value() const 
    { 
        float f = length();
        if( f > 0) 
            return ofVec3f(x/f, y/f, z/f); 
        else 
            return ofVec3f(0, 0, 0); 
    } 

	
	/**
     * Limit the length of this vector
     * @param max the maximum length to limit this vector by
     */  
	 
	 ofVec3f limit(float max) 
	 {
		if (length() > max) {
			normalize();
			x*=max; y*=max; z*=max;
		}
		return *this; 
	}
	
	
    /**
     * Rotate around axis.
     * @param ax, ay, az is the angle in DEGREES to rotate around.
     */
	void rotate(float ax, float ay, float az) 
	{
	    float a = cos(DEG_TO_RAD*(ax));
	    float b = sin(DEG_TO_RAD*(ax));
	    float c = cos(DEG_TO_RAD*(ay));
	    float d = sin(DEG_TO_RAD*(ay));
	    float e = cos(DEG_TO_RAD*(az));
	    float f = sin(DEG_TO_RAD*(az));
	    
	    float nx = c * e * x - c * f * y + d * z;
	    float ny = (a * f + b * d * e) * x + (a * e - b * d * f) * y - b * c * z;
	    float nz = (b * f - a * d * e) * x + (a * d * f + b * e) * y + a * c * z;
	    
	    x = nx; y = ny; z = nz;
		
		
	 }

	
};

#endif
