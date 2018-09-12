function v = helics_ok()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183032);
  end
  v = vInitialized;
end
