function v = helics_pending_time_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183059);
  end
  v = vInitialized;
end
