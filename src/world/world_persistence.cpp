#include "world_persistence.hpp"
#include "utils/files.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <iterator>
namespace vblck
{
namespace world
{
namespace persistence
{
constexpr const char* WORLDS_PATH = "worlds/";
constexpr size_t BLOCKS_PER_CHUNK =
	CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr size_t CHUNK_MEMORY_SIZE =
	sizeof(uint16_t) * BLOCKS_PER_CHUNK;
void createDb(const std::string& worldName)
{
	auto* world = World::get();
	SQLite::Database db(worldName,
						SQLite::OPEN_READWRITE |
							SQLite::OPEN_CREATE);
	auto createSql =
		loadTextFile("res/NewMap.sql");
	db.exec(createSql);

	SQLite::Statement query(
		db,
		"INSERT OR REPLACE INTO properties (ID, "
		"WorldSize, WorldHeight) "
		"VALUES (?, ?, ?)");
	query.bind(1, 1);
	query.bind(2, world->worldSize);
	query.bind(3, world->worldHeight);
	query.exec();
}

void loadChunk(const SQLite::Database& db,
			   uint32_t x,
			   uint32_t y,
			   uint32_t z)
{
	SQLite::Statement query(
		db,
		"SELECT block_data FROM chunks WHERE "
		"chunk_x = ? AND chunk_y = ? AND chunk_z "
		"= ?");

	query.bind(1, x);
	query.bind(2, y);
	query.bind(3, z);
	auto* chunk = World::get()->chunkAt(x, y, z);
	if(query.executeStep())
	{
		const void* blob =
			query.getColumn("block_data")
				.getBlob();
		int blob_size =
			query.getColumn("block_data")
				.getBytes();
		if(blob_size != CHUNK_MEMORY_SIZE)
		{
			std::abort();
		}
		memcpy(chunk->blocks.data(),
			   blob,
			   blob_size);
	}
	else
	{
		memset(chunk->blocks.data(),
			   0,
			   CHUNK_MEMORY_SIZE);
	}
}
void saveWorld(const std::string& worldName)
{
	auto* world = World::get();
	auto dbPath =
		WORLDS_PATH + worldName + ".sqlite3";
	if(!fileExists(dbPath))
	{
		createDirIfNotExists(WORLDS_PATH);
		createDb(dbPath);
	}
	SQLite::Database db(dbPath,
						SQLite::OPEN_READWRITE);
	db.exec("DELETE from chunks");

	for(uint32_t cx = 0; cx < world->worldSize;
		cx++)
	{
		for(uint32_t cz = 0;
			cz < world->worldSize;
			cz++)
		{
			for(uint32_t cy = 0;
				cy < world->worldHeight;
				cy++)
			{
				auto* chunk =
					world->chunkAt(cx, cy, cz);
				if(chunk->isEmpty())
				{
					continue;
				}
				SQLite::Statement query(
					db,
					"INSERT OR REPLACE INTO "
					"chunks (chunk_x, chunk_y, "
					"chunk_z, block_data) "
					"VALUES (?, ?, ?, ?)");

				query.bind(1, cx);
				query.bind(2, cy);
				query.bind(3, cz);
				query.bind(4,
						   chunk->blocks.data(),
						   CHUNK_MEMORY_SIZE);

				query.exec();
			}
		}
	}
}
void loadWorld(const std::string& worldName)
{
	auto* world = World::get();
	auto dbPath =
		WORLDS_PATH + worldName + ".sqlite3";
	SQLite::Database db(dbPath,
						SQLite::OPEN_READWRITE);

	SQLite::Statement query(
		db,
		"SELECT WorldSize, WorldHeight FROM "
		"properties WHERE ID = 1");
	uint32_t worldSize = 0, worldHeight = 0;
	if(query.executeStep())
	{
		worldSize = query.getColumn("WorldSize")
						.getUInt();
		worldHeight =
			query.getColumn("WorldHeight")
				.getUInt();
	}
	else
	{
		std::abort();
	}

	world->create(worldSize, worldHeight);

	for(uint32_t cx = 0; cx < world->worldSize;
		cx++)
	{
		for(uint32_t cz = 0;
			cz < world->worldSize;
			cz++)
		{
			for(uint32_t cy = 0;
				cy < world->worldHeight;
				cy++)
			{
				loadChunk(db, cx, cy, cz);
			}
		}
	}
}
bool exists(const ::std::string& worldName)
{
	return fileExists(WORLDS_PATH + worldName +
					  ".sqlite3");
}
} // namespace persistence
} // namespace world
} // namespace vblck