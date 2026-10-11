-- fireTame-heltec — schema inicial Supabase
-- Ejecutar en SQL Editor

create table if not exists public.nodes (
  id uuid primary key default gen_random_uuid(),
  name text not null,
  ttn_device_id text not null unique,
  deveui text not null unique,
  location_label text,
  lat double precision,
  lng double precision,
  status text not null default 'active'
    check (status in ('active', 'inactive', 'maintenance')),
  notes text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

create table if not exists public.sensor_readings (
  id bigserial primary key,
  node_id uuid not null references public.nodes(id) on delete cascade,
  received_at timestamptz not null,
  temp_dht_c real,
  humidity_pct real,
  mq135_adc integer,
  temp_bmp_c real,
  pressure_hpa real,
  pitch_deg real,
  roll_deg real,
  rssi integer,
  snr real,
  gateway_id text,
  f_port smallint default 2,
  schema_version smallint default 1,
  raw_payload jsonb,
  created_at timestamptz not null default now()
);

create index if not exists sensor_readings_node_time_idx
  on public.sensor_readings (node_id, received_at desc);

create table if not exists public.risk_events (
  id uuid primary key default gen_random_uuid(),
  node_id uuid not null references public.nodes(id) on delete cascade,
  reading_id bigint references public.sensor_readings(id) on delete set null,
  level text not null check (level in ('low', 'medium', 'high', 'critical')),
  score smallint not null check (score between 0 and 100),
  reason jsonb,
  created_at timestamptz not null default now()
);

create index if not exists risk_events_node_time_idx
  on public.risk_events (node_id, created_at desc);

insert into public.nodes (name, ttn_device_id, deveui, status) values
  ('Nodo 1', 'heltec-wifi-lora-32-v3', '70B3D57ED0079581', 'active'),
  ('Nodo 2', 'heltec-nodo-2', '70B3D57ED0079582', 'active'),
  ('Nodo 3', 'heltec-nodo-3', '70B3D57ED0079584', 'active')
on conflict (ttn_device_id) do update
  set deveui = excluded.deveui,
      name = excluded.name,
      updated_at = now();

-- Realtime (ignorar error si ya están en la publication)
alter publication supabase_realtime add table public.sensor_readings;
alter publication supabase_realtime add table public.risk_events;
