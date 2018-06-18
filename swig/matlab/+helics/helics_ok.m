function v = helics_ok()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176303);
  end
  v = vInitialized;
end
