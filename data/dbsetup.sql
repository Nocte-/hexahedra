CREATE TABLE height   (x INTEGER, y INTEGER, z INTEGER, timestamp INTEGER, PRIMARY KEY (x, y));
CREATE TABLE area     (x INTEGER, y INTEGER, idx INTEGER, data BLOB, PRIMARY KEY (x, y, idx));
CREATE TABLE chunk    (x INTEGER, y INTEGER, z INTEGER, data BLOB, timestamp INTEGER, PRIMARY KEY (x, y, z));
CREATE TABLE lightmap (x INTEGER, y INTEGER, z INTEGER, data BLOB, timestamp INTEGER, PRIMARY KEY (x, y, z));
CREATE TABLE surface  (x INTEGER, y INTEGER, z INTEGER, data BLOB, timestamp INTEGER, PRIMARY KEY (x, y, z));

CREATE TABLE component(id INTEGER, name VARCHAR NOT NULL, INTEGER type NOT NULL, PRIMARY KEY(id));
CREATE TABLE entity   (id INTEGER, componentid INTEGER, val BLOB, PRIMARY KEY(id, componentid), FOREIGN KEY (componentid) REFERENCES component(id));
CREATE TABLE prototype(id VARCHAR, entityid INTEGER, PRIMARY KEY(id), FOREIGN KEY (entityid) REFERENCES entity(id));

