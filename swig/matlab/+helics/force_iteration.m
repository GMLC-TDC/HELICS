function v = force_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783847);
  end
  v = vInitialized;
end
