CREATE TABLE height (x INTEGER, y INTEGER, z INTEGER, timestamp INTEGER, PRIMARY KEY (x, y));
CREATE TABLE surface (x INTEGER, y INTEGER, z INTEGER, opaque BLOB, transparent BLOB, timestamp INTEGER, PRIMARY KEY (x, y, z));
CREATE TABLE lightsurface (x INTEGER, y INTEGER, z INTEGER, opaque BLOB, transparent BLOB, timestamp INTEGER, PRIMARY KEY (x, y, z));

