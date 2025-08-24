CREATE TABLE IF NOT EXISTS chunks (
    chunk_x INTEGER NOT NULL,
    chunk_y INTEGER NOT NULL, 
    chunk_z INTEGER NOT NULL,

    block_data BLOB NOT NULL,
    
    PRIMARY KEY (chunk_x, chunk_y, chunk_z)
);

CREATE TABLE IF NOT EXISTS properties(
    ID INTEGER NOT NULL,

    WorldSize INTEGER,
    WorldHeight INTEGER,

    PRIMARY KEY (ID) 
);