-- Pour chaque rue trouve les zones administratives qui le contiennent
CREATE OR REPLACE FUNCTION georef.match_way_to_admin() RETURNS VOID AS $$
INSERT INTO georef.rel_way_admin(way_id, admin_id)
SELECT DISTINCT way.id, ad.id
FROM
	georef.way way,
	georef.admin ad,
    georef.edge edge
WHERE
    edge.way_id = way.id
	AND st_within(edge.the_geog::geometry, ad.boundary::geometry)
	AND ad.boundary && edge.the_geog -- On force l’utilisation des indexes spatiaux en comparant les bounding boxes
$$
LANGUAGE SQL;

-- Chargement de la table georef.fusion_ways
CREATE OR REPLACE FUNCTION georef.fusion_ways_by_admin_name() RETURNS VOID AS $$
delete from georef.fusion_ways; -- Netoyage de la table
insert into georef.fusion_ways(id, way_id)
select test2.id, test1.way_id from
(
SELECT a.id as admin_id, w.name as way_name, w.id as way_id
  FROM georef.way w, georef.rel_way_admin r, georef.admin a
  where w.id=r.way_id
  and r.admin_id=a.id
  and a.level=8 -- On choisit que les communes
  group by w.name, a.id, w.id
  order by a.id, w.name, w.id) test1
  inner join (
  SELECT a.id as admin_id, w.name as way_name, min(w.id) as id
  FROM georef.way w, georef.rel_way_admin r, georef.admin a
  where w.id=r.way_id
  and r.admin_id=a.id
  and a.level=8
  group by a.id, w.name
  order by a.id, w.name)test2
  on(test1.admin_id = test2.admin_id and test1.way_name = test2.way_name)
$$
LANGUAGE SQL;

-- MAJ des way_id de la table georef.house_number
-- See also http://workshops.boundlessgeo.com/postgis-intro/knn.html
CREATE OR REPLACE FUNCTION georef.nearest_way_id(point_in GEOGRAPHY(POINT, 4326))
  RETURNS bigint AS
$func$
BEGIN
RETURN (
WITH closest_candidates AS (
  SELECT
    ed.way_id,
    ed.the_geog
  FROM
    georef.edge ed
  ORDER BY ed.the_geog::geometry <-> point_in::geometry
  LIMIT 100
)
SELECT way_id
FROM closest_candidates
ORDER BY ST_Distance(the_geog, point_in)
LIMIT 1
);
END
$func$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION georef.update_house_number() RETURNS VOID AS $$
UPDATE georef.house_number set way_id=georef.nearest_way_id(coord);
$$
LANGUAGE SQL;

-- MAJ des way_id de la table georef.edge
CREATE OR REPLACE FUNCTION georef.update_edge() RETURNS VOID AS $$
update georef.edge ge
set way_id=fw.id
from georef.fusion_ways fw
where ge.way_id=fw.way_id
and fw.id <> fw.way_id
and ge.way_id <> fw.id

$$
LANGUAGE SQL;

-- MAJ de la table .. pour supprimer les voies plus tard
CREATE OR REPLACE FUNCTION georef.add_fusion_ways() RETURNS VOID AS $$
insert into georef.fusion_ways
select w.id, w.id from georef.way w
left outer join georef.rel_way_admin a
on (w.id = a.way_id)
where a.admin_id is NULL
$$
LANGUAGE SQL;
-- Ajout des voies qui ne sont pas dans la table de fusion : cas voie appartient à un seul admin avec un level 9
CREATE OR REPLACE FUNCTION georef.complete_fusion_ways() RETURNS VOID AS $$
insert into georef.fusion_ways(id, way_id)
select w.id, w.id from georef.way w
left outer join georef.fusion_ways fw
on(w.id=fw.way_id)
where fw.id IS NULL
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION georef.add_way_name() RETURNS VOID AS $$
UPDATE georef.way
SET name = 'NC:' || CAST(id as varchar)
where name = ''
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION georef.clean_way() RETURNS VOID AS $$
delete from georef.way
using georef.fusion_ways
where georef.way.id = georef.fusion_ways.id and
georef.fusion_ways.id IS NULL
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION georef.clean_way_name() RETURNS VOID AS $$
update georef.way set name='' where name like 'NC:%'
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION georef.insert_tmp_rel_way_admin() RETURNS VOID AS $$
delete from georef.tmp_rel_way_admin;
insert into georef.tmp_rel_way_admin select distinct rwa.admin_id, fw.id from georef.rel_way_admin rwa, georef.fusion_ways fw
where rwa.way_id=fw.way_id
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION georef.insert_rel_way_admin() RETURNS VOID AS $$
delete from georef.rel_way_admin;
insert into georef.rel_way_admin select * from georef.tmp_rel_way_admin
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION georef.update_admin_coord() RETURNS VOID AS $$
UPDATE georef.admin set coord = st_centroid(boundary::geometry)
where st_X(coord::geometry)=0
and st_y(coord::geometry)=0
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION georef.match_admin_to_admin() RETURNS VOID AS $$
INSERT INTO georef.rel_admin_admin(master_admin_id, admin_id)
select distinct ad1.id, ad2.id
from georef.admin ad1, georef.admin ad2
WHERE st_within(ad2.coord::geometry, ad1.boundary::geometry)
AND ad2.coord && ad1.boundary -- On force l’utilisation des indexes spatiaux en comparant les bounding boxes
and ad1.id <> ad2.id
and ad2.level > ad1.level
$$
LANGUAGE SQL;

 -- Conversion des coordonnées vers wgs84
CREATE OR REPLACE FUNCTION georef.coord2wgs84(lon real, lat real, coord_in int)
  RETURNS RECORD AS
$$
DECLARE 
  ret RECORD;
BEGIN
	SELECT ST_X(aa.new_coord::geometry) as lon, ST_Y(aa.new_coord::geometry) as lat from (
	SELECT ST_Transform(ST_GeomFromText('POINT('||lon||' '||lat||')',coord_in),4326) As new_coord)aa INTO ret;
	RETURN ret;
END
$$ 
LANGUAGE plpgsql;

-- fonction permettant de mettre à jour les boundary des admins
CREATE OR REPLACE FUNCTION georef.update_boundary(adminid bigint)
  RETURNS VOID AS
$$
DECLARE 
	ret GEOGRAPHY;
BEGIN
	SELECT ST_Multi(ST_ConvexHull(ST_Collect(
		ARRAY(
				select aa.coord::geometry from (
					select n.coord as coord from georef.rel_way_admin rel,
						georef.edge e, georef.node n
					where rel.admin_id=adminid
					and rel.way_id=e.way_id
					and e.source_node_id=n.id
					UNION
					select n.coord as coord from georef.rel_way_admin rel,
						georef.edge e, georef.node n
					where rel.admin_id=adminid
					and rel.way_id=e.way_id
					and e.target_node_id=n.id)aa)))) INTO ret;
	CASE  geometrytype(ret::geometry) 
		WHEN 'MULTIPOLYGON'  THEN
			UPDATE georef.admin 
			set boundary = ret::geometry
			where georef.admin.id=adminid;
		WHEN 'MultiLineString'  THEN
			UPDATE georef.admin 
			set boundary = ST_Multi(ST_Polygonize(ret::geometry))
			where georef.admin.id=adminid;
		ELSE 
			UPDATE georef.admin 
			set boundary = NULL
			where georef.admin.id=adminid;
	END CASE;
END
$$
LANGUAGE plpgsql;

