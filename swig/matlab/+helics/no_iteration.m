function v = no_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783846);
  end
  v = vInitialized;
end
