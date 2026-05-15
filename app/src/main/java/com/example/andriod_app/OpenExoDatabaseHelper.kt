package com.example.andriod_app

import android.content.ContentValues
import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper

/**
 * SQLite key-value store aligned with iOS `OpenExoDatabase` / Python `gui_settings.txt` + controller snapshot.
 *
 * Keys:
 * - [KEY_GUI_SETTINGS]: JSON object for GUI prefs (bilateral, last joint/controller selections, FSR indices, CSV prefix, …)
 * - [KEY_CONTROLLER_SNAPSHOT]: JSON for `{ deviceUUID, matrix, values, parameterNames, updatedAt }`
 * - [KEY_MIGRATED_FROM_USER_DEFAULTS]: `"true"` after one-time import from SharedPreferences (optional)
 *
 * Wire your Compose/UI layer to serialize/deserialize JSON into the same field names as iOS `GUISettings`
 * and `ControllerSnapshot`.
 */
class OpenExoDatabaseHelper(context: Context) :
    SQLiteOpenHelper(context.applicationContext, DB_NAME, null, VERSION) {

    override fun onCreate(db: SQLiteDatabase) {
        db.execSQL(
            """
            CREATE TABLE kv (
                key TEXT PRIMARY KEY NOT NULL,
                value TEXT NOT NULL
            );
            """.trimIndent(),
        )
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        db.execSQL("DROP TABLE IF EXISTS kv")
        onCreate(db)
    }

    fun putJson(key: String, json: String) {
        val db = writableDatabase
        val cv = ContentValues().apply {
            put(COL_KEY, key)
            put(COL_VALUE, json)
        }
        db.insertWithOnConflict(TABLE_KV, null, cv, SQLiteDatabase.CONFLICT_REPLACE)
    }

    fun getJson(key: String): String? {
        val db = readableDatabase
        val c = db.rawQuery(
            "SELECT $COL_VALUE FROM $TABLE_KV WHERE $COL_KEY = ? LIMIT 1",
            arrayOf(key),
        )
        return try {
            if (c.moveToFirst()) c.getString(0) else null
        } finally {
            c.close()
        }
    }

    fun deleteKey(key: String) {
        writableDatabase.delete(TABLE_KV, "$COL_KEY = ?", arrayOf(key))
    }

    companion object {
        private const val DB_NAME = "openexo.sqlite"
        private const val VERSION = 1
        private const val TABLE_KV = "kv"
        private const val COL_KEY = "key"
        private const val COL_VALUE = "value"

        const val KEY_GUI_SETTINGS = "gui_settings_v1"
        const val KEY_CONTROLLER_SNAPSHOT = "controller_snapshot_v1"
        const val KEY_MIGRATED_FROM_USER_DEFAULTS = "migrated_from_user_defaults_v1"
    }
}
