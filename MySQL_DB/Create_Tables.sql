 /*
    This file is part of RSSReaderServer.

    RSSReaderServer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RSSReaderServer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RSSReaderServer.  If not, see <https://www.gnu.org/licenses/>.
*/

 USE rssreader;

 DROP EVENT IF EXISTS deleteOldSessionsAtMidnight;
 DROP PROCEDURE IF EXISTS deleteOldSessions;
 DROP TABLE IF EXISTS links;
 DROP TABLE IF EXISTS navigators;
 DROP TABLE IF EXISTS users;
 DROP TABLE IF EXISTS linkCache;

 CREATE TABLE users (
	email VARCHAR(256) NOT NULL,
    emailConfirmed BOOL DEFAULT 0,
    userName VARCHAR(128) NOT NULL,
	userPassword VARCHAR(64),
    rSalt VARCHAR(16),
    idGoogle VARCHAR(36), -- SUB - User ID
    settings MEDIUMTEXT,
    othersInfo MEDIUMTEXT, -- Internal information for otheres
    linkPhoto VARCHAR(2048),
    subsDate DATETIME DEFAULT CURRENT_TIMESTAMP, -- Subscription date
    PRIMARY KEY (email)
)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE navigators (
	email VARCHAR(256) NOT NULL,
    CONSTRAINT `nav_usr_email` FOREIGN KEY (email) REFERENCES users (email) ON DELETE CASCADE ON UPDATE CASCADE,
    uuid VARCHAR(36),
    hashNavigator VARCHAR(64),
    lastAccess DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (email, uuid)
)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE links (
	email VARCHAR(256) NOT NULL,
    CONSTRAINT `links_usr_email` FOREIGN KEY (email) REFERENCES users (email) ON DELETE CASCADE ON UPDATE CASCADE,
    link VARCHAR(2048) NOT NULL,
    linkName VARCHAR(64) NOT NULL,
    category MEDIUMTEXT,
    dateAdded DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY(email, link(512))
)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE linkCache (
	link VARCHAR(2048) NOT NULL,
    content MEDIUMBLOB,
    expirationDate DATETIME,
    PRIMARY KEY (link(768))
)ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- This procedure and event were created by Pedro Carvalho (https://github.com/inceptusp)

DELIMITER $$
CREATE PROCEDURE deleteOldSessions ()
BEGIN
	DELETE FROM rssreader.navigators WHERE lastAccess < CURDATE() - INTERVAL 7 DAY;
END $$
DELIMITER ;

CREATE EVENT deleteOldSessionsAtMidnight ON SCHEDULE EVERY 1 DAY 
	STARTS CONCAT(DATE(NOW()+INTERVAL 1 DAY ), ' 00:00:00') DO
		CALL deleteOldSessions();

DELIMITER $$
CREATE PROCEDURE deleteOldCache ()
BEGIN
	DELETE FROM rssreader.linkCache WHERE expirationDate < CURDATE() - INTERVAL 15 DAY;
END $$
DELIMITER ;

CREATE EVENT deleteOldCacheAt1AM ON SCHEDULE EVERY 1 DAY 
	STARTS CONCAT(DATE(NOW()+INTERVAL 1 DAY ), ' 01:00:00') DO
		CALL deleteOldCache();